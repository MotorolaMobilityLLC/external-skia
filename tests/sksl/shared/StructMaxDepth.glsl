
struct S1 {
    int x;
};
struct S2 {
    S1 x;
};
struct S3 {
    S2 x;
};
struct S4 {
    S3 x;
};
struct S5 {
    S4 x;
};
struct S6 {
    S5 x;
};
struct S7 {
    S6 x;
};
struct S8 {
    S7 x;
};
in S8 s8;
struct SA1 {
    int x[2];
};
struct SA2 {
    SA1 x[2];
};
struct SA3 {
    SA2 x[2];
};
struct SA4 {
    SA3 x[2];
};
struct SA5 {
    SA4 x[2];
};
struct SA6 {
    SA5 x[2];
};
struct SA7 {
    SA6 x[2];
};
struct SA8 {
    SA7 x[2];
};
in SA8 sa8[2];
