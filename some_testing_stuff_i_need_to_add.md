static void subset(H3Index* set1, int len1, H3Index* set2, int len2) {
    int isthere;

    for (int i = 0; i < len1; i++) {
        if (set1[i] == 0) continue;

        isthere = 0;
        for (int j = 0; j < len2; j++) {
            if (set1[i] == set2[j]) isthere++;
        }
        t_assert(isthere == 1, "children must match");
    }
}

static void sets_equal(H3Index* set1, int len1, H3Index* set2, int len2) {
    subset(set1, len1, set2, len2);
    subset(set2, len2, set1, len1);
}



    TEST(pentagonChildren) {
        H3Index pentagon = 0x81083ffffffffff;  // res 1 pentagon
        const int childRes = 3;

        const int expectedCount = (5 * 7) + 6;
        const int paddedCount =
            H3_EXPORT(maxH3ToChildrenSize)(pentagon, childRes);

        H3Index expectedCells[] = {
            0x830800fffffffff, 0x830802fffffffff, 0x830803fffffffff,
            0x830804fffffffff, 0x830805fffffffff, 0x830806fffffffff,
            0x830810fffffffff, 0x830811fffffffff, 0x830812fffffffff,
            0x830813fffffffff, 0x830814fffffffff, 0x830815fffffffff,
            0x830816fffffffff, 0x830818fffffffff, 0x830819fffffffff,
            0x83081afffffffff, 0x83081bfffffffff, 0x83081cfffffffff,
            0x83081dfffffffff, 0x83081efffffffff, 0x830820fffffffff,
            0x830821fffffffff, 0x830822fffffffff, 0x830823fffffffff,
            0x830824fffffffff, 0x830825fffffffff, 0x830826fffffffff,
            0x830828fffffffff, 0x830829fffffffff, 0x83082afffffffff,
            0x83082bfffffffff, 0x83082cfffffffff, 0x83082dfffffffff,
            0x83082efffffffff, 0x830830fffffffff, 0x830831fffffffff,
            0x830832fffffffff, 0x830833fffffffff, 0x830834fffffffff,
            0x830835fffffffff, 0x830836fffffffff};

        H3Index* children = calloc(paddedCount, sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(pentagon, childRes, children);

        sets_equal(children, paddedCount, expectedCells, expectedCount);
