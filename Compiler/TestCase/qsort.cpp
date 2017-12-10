//#include <cstdio>
int a[100];
int partition(int p, int r)
{
    int i, j, t;
    i = p - 1;
    j = p;
    while (j <= r - 1)
    {
        if (a[j] < a[r])
        {
            i = i + 1;
            t = a[i];
            a[i] = a[j];
            a[j] = t;
        }
        else
            ;
        j = j + 1;
    }
    t = a[i + 1];
    a[i + 1] = a[r];
    a[r] = t;
    return (i + 1);
}
void qsort(int p, int r)
{
    int q;
    if (p < r)
    {
        q = partition(p, r);
        qsort(p, q - 1);
        qsort(q + 1, r);
    }
    else
        ;
}
//int main(
void main()
{
    int i, j;
    j = 10;
    i = j - 1;
    while (i >= 0)
    {
        a[j - 1 - i] = i;
        i = i - 1;
    }
    qsort(0, j - 1);
    i = 0;
    while (i < j)
    {
        // printf("%d", a[i]);
        printf(a[i]);
        i = i + 1;
    }
}