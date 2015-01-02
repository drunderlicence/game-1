#if !defined(BIG_NUMBERS_H)

#define BIG_NUMBER_WIDTH 5
#define BIG_NUMBER_HEIGHT 6

static char BN_0[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    0,1,1,1,0,
    1,0,0,0,1,
    1,0,0,0,1,
    1,0,0,0,1,
    1,0,0,0,1,
    0,1,1,1,0,
};

static char BN_1[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    0,0,1,0,0,
    0,1,1,0,0,
    0,0,1,0,0,
    0,0,1,0,0,
    0,0,1,0,0,
    0,1,1,1,0,
};

static char BN_2[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    0,1,1,1,0,
    1,0,0,0,1,
    0,0,0,1,0,
    0,0,1,0,0,
    0,1,0,0,0,
    1,1,1,1,1,
};

static char BN_3[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    0,1,1,1,0,
    1,0,0,0,1,
    0,0,1,1,0,
    0,0,0,0,1,
    1,0,0,0,1,
    0,1,1,1,0,
};

static char BN_4[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    1,0,0,1,0,
    1,0,0,1,0,
    1,0,0,1,0,
    1,1,1,1,1,
    0,0,0,1,0,
    0,0,0,1,0,
};

static char BN_5[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    1,1,1,1,1,
    1,0,0,0,0,
    0,1,1,1,0,
    0,0,0,0,1,
    1,0,0,0,1,
    0,1,1,1,0,
};

static char BN_6[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    0,0,1,1,1,
    0,1,0,0,0,
    1,1,1,1,0,
    1,0,0,0,1,
    1,0,0,0,1,
    0,1,1,1,0,
};

static char BN_7[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    1,1,1,1,1,
    0,0,0,0,1,
    0,0,0,1,0,
    0,0,1,0,0,
    0,0,1,0,0,
    0,0,1,0,0,
};

static char BN_8[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    0,1,1,1,0,
    1,0,0,0,1,
    0,1,1,1,0,
    1,0,0,0,1,
    1,0,0,0,1,
    0,1,1,1,0,
};

static char BN_9[BIG_NUMBER_WIDTH * BIG_NUMBER_HEIGHT] =
{
    0,1,1,1,0,
    1,0,0,0,1,
    1,0,0,0,1,
    0,1,1,1,1,
    0,0,0,1,0,
    1,1,1,0,0,
};

static char *BN[10] =
{
    BN_0,
    BN_1,
    BN_2,
    BN_3,
    BN_4,
    BN_5,
    BN_6,
    BN_7,
    BN_8,
    BN_9,
};


#define BIG_NUMBERS_H
#endif
