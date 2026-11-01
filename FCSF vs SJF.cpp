#include <stdio.h>
#include <string.h>
 
#define MAX_PROSES 20
#define MAX_SEGMEN (MAX_PROSES * 2 + 2)
 
typedef struct {
    int pid;   /* ID proses            */
    int at;    /* Arrival Time         */
    int bt;    /* Burst Time           */
    int ct;    /* Completion Time      */
    int tat;   /* Turnaround Time      */
    int wt;    /* Waiting Time         */
    int done;  /* penanda sudah selesai */
} Proses;
 
typedef struct {
    int pid;   /* -1 = CPU idle */
    int start;
    int end;
} Segmen;
 
typedef struct {
    Segmen gantt[MAX_SEGMEN];
    int jml_segmen;
    double avg_wt;
    double avg_tat;
} Hasil;
 
/* ---------- Utilitas ---------- */
 
static void tambah_segmen(Hasil *h, int pid, int start, int end) {
    /* gabungkan jika segmen sebelumnya sama (misal idle berurutan) */
    if (h->jml_segmen > 0 && h->gantt[h->jml_segmen - 1].pid == pid &&
        h->gantt[h->jml_segmen - 1].end == start) {
        h->gantt[h->jml_segmen - 1].end = end;
        return;
    }
    h->gantt[h->jml_segmen].pid = pid;
    h->gantt[h->jml_segmen].start = start;
    h->gantt[h->jml_segmen].end = end;
    h->jml_segmen++;
}
 
static void hitung_rata2(Proses p[], int n, Hasil *h) {
    double total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
        total_wt += p[i].wt;
        total_tat += p[i].tat;
    }
    h->avg_wt = total_wt / n;
    h->avg_tat = total_tat / n;
}
 
/* ---------- Algoritma FCFS ---------- */
 
static void simulasi_fcfs(Proses p[], int n, Hasil *h) {
    int urut[MAX_PROSES];
    for (int i = 0; i < n; i++) urut[i] = i;
 
    /* urutkan indeks berdasarkan arrival time (tie-break: PID) */
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++) {
            Proses *a = &p[urut[j]], *b = &p[urut[j + 1]];
            if (a->at > b->at || (a->at == b->at && a->pid > b->pid)) {
                int t = urut[j]; urut[j] = urut[j + 1]; urut[j + 1] = t;
            }
        }
 
    int waktu = 0;
    h->jml_segmen = 0;
    for (int i = 0; i < n; i++) {
        Proses *cur = &p[urut[i]];
        if (waktu < cur->at) {                 /* CPU idle */
            tambah_segmen(h, -1, waktu, cur->at);
            waktu = cur->at;
        }
        tambah_segmen(h, cur->pid, waktu, waktu + cur->bt);
        waktu += cur->bt;
        cur->ct = waktu;
    }
    hitung_rata2(p, n, h);
}
 
/* ---------- Algoritma SJF (Non-Preemptive) ---------- */
 
static void simulasi_sjf(Proses p[], int n, Hasil *h) {
    int waktu = 0, selesai = 0;
    h->jml_segmen = 0;
    for (int i = 0; i < n; i++) p[i].done = 0;
 
    while (selesai < n) {
        int pilih = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].done || p[i].at > waktu) continue;
            if (pilih == -1 || p[i].bt < p[pilih].bt ||
                (p[i].bt == p[pilih].bt && p[i].at < p[pilih].at) ||
                (p[i].bt == p[pilih].bt && p[i].at == p[pilih].at &&
                 p[i].pid < p[pilih].pid))
                pilih = i;
        }
 
        if (pilih == -1) {                     /* belum ada proses datang -> idle */
            int next = -1;
            for (int i = 0; i < n; i++)
                if (!p[i].done && (next == -1 || p[i].at < next)) next = p[i].at;
            tambah_segmen(h, -1, waktu, next);
            waktu = next;
            continue;
        }
 
        tambah_segmen(h, p[pilih].pid, waktu, waktu + p[pilih].bt);
        waktu += p[pilih].bt;
        p[pilih].ct = waktu;
        p[pilih].done = 1;
        selesai++;
    }
    hitung_rata2(p, n, h);
}
 
/* ---------- Tampilan ---------- */
 
static void cetak_gantt(const Hasil *h) {
    printf("\nGantt Chart:\n");
    for (int i = 0; i < h->jml_segmen; i++) {
        if (h->gantt[i].pid == -1) printf("| IDLE ");
        else printf("|  P%d  ", h->gantt[i].pid);
    }
    printf("|\n");
    printf("%d", h->gantt[0].start);
    for (int i = 0; i < h->jml_segmen; i++) printf("      %d", h->gantt[i].end);
    printf("\n");
}
 
static void cetak_tabel(const Proses p[], int n, const Hasil *h, const char *judul) {
    printf("\n==================== %s ====================\n", judul);
    printf("%-5s %-8s %-8s %-8s %-8s %-8s\n", "PID", "Arrival", "Burst", "CT", "TAT", "WT");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < n; i++)
        printf("P%-4d %-8d %-8d %-8d %-8d %-8d\n",
               p[i].pid, p[i].at, p[i].bt, p[i].ct, p[i].tat, p[i].wt);
    printf("-----------------------------------------------------\n");
    printf("Rata-rata Waiting Time    : %.2f\n", h->avg_wt);
    printf("Rata-rata Turnaround Time : %.2f\n", h->avg_tat);
    cetak_gantt(h);
}
 
static void cetak_perbandingan(const Hasil *f, const Hasil *s) {
    printf("\n==================== PERBANDINGAN ====================\n");
    printf("%-28s %-10s %-10s\n", "Metrik", "FCFS", "SJF");
    printf("%-28s %-10.2f %-10.2f\n", "Rata-rata Waiting Time", f->avg_wt, s->avg_wt);
    printf("%-28s %-10.2f %-10.2f\n", "Rata-rata Turnaround Time", f->avg_tat, s->avg_tat);
    printf("------------------------------------------------------\n");
 
    if (s->avg_wt < f->avg_wt)
        printf("Kesimpulan: SJF lebih efisien (WT lebih kecil %.2f satuan waktu).\n",
               f->avg_wt - s->avg_wt);
    else if (s->avg_wt > f->avg_wt)
        printf("Kesimpulan: FCFS lebih efisien (WT lebih kecil %.2f satuan waktu).\n",
               s->avg_wt - f->avg_wt);
    else
        printf("Kesimpulan: FCFS dan SJF menghasilkan performa yang sama.\n");
}
 
/* ---------- Input ---------- */
 
static int input_manual(Proses p[]) {
    int n;
    do {
        printf("Jumlah proses (1-%d): ", MAX_PROSES);
        if (scanf("%d", &n) != 1) { while (getchar() != '\n'); n = 0; }
    } while (n < 1 || n > MAX_PROSES);
 
    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        do {
            printf("P%d - Arrival Time & Burst Time (contoh: 0 5): ", p[i].pid);
            if (scanf("%d %d", &p[i].at, &p[i].bt) != 2) {
                while (getchar() != '\n');
                p[i].at = -1; p[i].bt = 0;
            }
        } while (p[i].at < 0 || p[i].bt <= 0);
    }
    return n;
}
 
static int data_contoh(Proses p[]) {
    int at[] = {0, 1, 2, 3, 4};
    int bt[] = {8, 4, 9, 5, 2};
    int n = 5;
    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        p[i].at = at[i];
        p[i].bt = bt[i];
    }
    return n;
}
 
int main(void) {
    Proses data[MAX_PROSES], fcfs[MAX_PROSES], sjf[MAX_PROSES];
    Hasil hf, hs;
    int pilihan, n;
 
    printf("=== SIMULASI PENJADWALAN CPU: FCFS vs SJF ===\n");
    printf("1. Gunakan data contoh\n");
    printf("2. Input data manual\n");
    printf("Pilih: ");
    if (scanf("%d", &pilihan) != 1) pilihan = 1;
 
    n = (pilihan == 2) ? input_manual(data) : data_contoh(data);
 
    memcpy(fcfs, data, sizeof(Proses) * n);
    memcpy(sjf, data, sizeof(Proses) * n);
 
    simulasi_fcfs(fcfs, n, &hf);
    simulasi_sjf(sjf, n, &hs);
 
    cetak_tabel(fcfs, n, &hf, "FCFS");
    cetak_tabel(sjf, n, &hs, "SJF (Non-Preemptive)");
    cetak_perbandingan(&hf, &hs);
 
    return 0;
}