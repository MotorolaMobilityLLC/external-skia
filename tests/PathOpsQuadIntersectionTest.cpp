/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkPathOpsRect.h"
#include "src/pathops/SkReduceOrder.h"
#include "tests/PathOpsQuadIntersectionTestData.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

static void standardTestCases(skiatest::Reporter* reporter) {
    bool showSkipped = false;
    for (size_t index = 0; index < quadraticTests_count; ++index) {
        const QuadPts& q1 = quadraticTests[index][0];
        SkDQuad quad1;
        quad1.debugSet(q1.fPts);
        SkASSERT(ValidQuad(quad1));
        const QuadPts& q2 = quadraticTests[index][1];
        SkDQuad quad2;
        quad2.debugSet(q2.fPts);
        SkASSERT(ValidQuad(quad2));
        SkReduceOrder reduce1, reduce2;
        int order1 = reduce1.reduce(quad1);
        int order2 = reduce2.reduce(quad2);
        if (order1 < 3) {
            if (showSkipped) {
                SkDebugf("[%d] quad1 order=%d\n", static_cast<int>(index), order1);
            }
        }
        if (order2 < 3) {
            if (showSkipped) {
                SkDebugf("[%d] quad2 order=%d\n", static_cast<int>(index), order2);
            }
        }
        if (order1 == 3 && order2 == 3) {
            SkIntersections intersections;
            intersections.intersect(quad1, quad2);
            if (intersections.used() > 0) {
                for (int pt = 0; pt < intersections.used(); ++pt) {
                    double tt1 = intersections[0][pt];
                    SkDPoint xy1 = quad1.ptAtT(tt1);
                    double tt2 = intersections[1][pt];
                    SkDPoint xy2 = quad2.ptAtT(tt2);
                    if (!xy1.approximatelyEqual(xy2)) {
                        SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                                __FUNCTION__, static_cast<int>(index), pt, tt1, xy1.fX, xy1.fY,
                                tt2, xy2.fX, xy2.fY);
                        REPORTER_ASSERT(reporter, 0);
                    }
                }
            }
        }
    }
}

static const QuadPts testSet[] = {
{{{123.637985f, 102.405312f}, {125.172699f, 104.575714f}, {123.387383f, 106.91227f}}},
{{{123.388428f, 106.910896f}, {123.365623f, 106.94088f}, {123.320007f, 107.000946f}}},

{{{-0.001019871095195412636, -0.008523519150912761688}, {-0.005396408028900623322, -0.005396373569965362549}, {-0.02855382487177848816, -0.02855364233255386353}}},
{{{-0.004567248281091451645, -0.01482933573424816132}, {-0.01142475008964538574, -0.01140109263360500336}, {-0.02852955088019371033, -0.02847047336399555206}}},

{{{1, 1}, {0, 2}, {3, 3}}},
{{{3, 0}, {0, 1}, {1, 2}}},

{{{0.33333333333333326, 0.81481481481481488}, {0.63395173631977997, 0.68744136726313931}, {1.205684411948591, 0.81344322326274499}}},
{{{0.33333333333333326, 0.81481481481481488}, {0.63396444791444551, 0.68743368362444768}, {1.205732763658403, 0.81345617746834109}}},

{{{4981.9990234375, 1590}, {4981.9990234375, 1617.7523193359375}, {4962.375, 1637.3760986328125}}},
{{{4962.3759765625, 1637.3760986328125}, {4982, 1617.7523193359375}, {4982, 1590}}},

{{{48.7416f, 7.74160004f}, {96.4831848f, -40}, {164, -40}}},
{{{56.9671326f, 0}, {52.7835083f, 3.69968891f}, {48.7416f, 7.74160004f}}},

{{{138, 80}, {147.15692138671875, 80}, {155.12803649902344, 82.86279296875}}},
{{{155.12803649902344, 82.86279296875}, {153.14971923828125, 82.152290344238281}, {151.09841918945312, 81.618133544921875}}},

{{{88, 130}, {88, 131.54483032226562}, {88.081489562988281, 133.0560302734375}}},
{{{88.081489562988281, 133.0560302734375}, {88, 131.54483032226562}, {88, 130}}},

{{{0.59987992,2.14448452}, {0.775417507,1.95606446}, {1.00564098,1.79310346}}},
{{{1.00564098,1.79310346}, {1.25936198,1.615623}, {1.35901463,1.46834028}}},

{{{3,0}, {0,1}, {3,2}}},
{{{2,0}, {1,1}, {2,2}}},

{{{38.656852722167969, 38.656852722167969}, {38.651023864746094, 38.662681579589844}, {38.644744873046875, 38.668937683105469}}},
{{{38.656852722167969, 38.656852722167969}, {36.313709259033203, 41}, {33, 41}}},

{{{4914.9990234375, 1523}, {4942.75146484375, 1523}, {4962.375, 1542.6239013671875}}},
{{{4962.3759765625, 1542.6239013671875}, {4942.75244140625, 1523}, {4915, 1523}}},

{{{4867.623046875, 1637.3760986328125}, {4847.9990234375, 1617.7523193359375}, {4847.9990234375, 1590}}},
{{{4848, 1590}, {4848, 1617.7523193359375}, {4867.6240234375, 1637.3760986328125}}},

{{{102.64466094970703, 165.3553466796875}, {110.79246520996094, 173.50314331054687}, {120.81797790527344, 177.11778259277344}}},
{{{113.232177734375, 173.57899475097656}, {116.88026428222656, 175.69805908203125}, {120.81797790527344, 177.11778259277344}}},

{{{-37.3484879,10.0192947}, {-36.4966316,13.2140198}, {-38.1506348,16.0788383}}},
{{{-38.1462746,16.08918}, {-36.4904327,13.2193804}, {-37.3484879,10.0192947}}},

{{{-37.3513985,10.0082998}, {-36.4938011,13.2090998}, {-38.1506004,16.0788002}}},
{{{-37.3508987,10.0102997}, {-36.4930992,13.2110004}, {-38.1497993,16.0809002}}},

{{{-37.3508987,10.0102997}, {-37.3510017,10.0098}, {-37.3512001,10.0093002}}},
{{{-49.0778008,19.0097008}, {-38.2086983,6.80954981}, {-37.3508987,10.0102997}}},

{{{SkBits2Float(0xc22423b2), SkBits2Float(0x40afae2c)},
        {SkBits2Float(0xc2189b24), SkBits2Float(0x40e3f058)},
        {SkBits2Float(0xc21511d9), SkBits2Float(0x41251125)}}},
{{{SkBits2Float(0xc2153d2f), SkBits2Float(0x412299db)},
        {SkBits2Float(0xc2153265), SkBits2Float(0x41233845)},
        {SkBits2Float(0xc21527fc), SkBits2Float(0x4123d684)}}},

{{{-37.3097496, 10.1625624}, {-37.2992134, 10.2012377}, {-37.2890472, 10.239872}}},
{{{-41.0348587, 5.49001122}, {-38.1515045, 7.12308884}, {-37.2674294, 10.3166857}}},

{{{-52.8062439,14.1493912}, {-53.6638947,10.948595}, {-52.0070419,8.07883835}}},
{{{-52.8054848,14.1522331}, {-53.6633072,10.9514809}, {-52.0066071,8.08163643}}},

{{{441.853149, 308.209106}, {434.672272, 315.389984}, {424.516998, 315.389984}}},
{{{385.207275, 334.241272}, {406.481598, 312.96698}, {436.567993, 312.96698}}},

{{{-708.00779269310044, -154.36998607290101}, {-707.90560262312511, -154.36998607290101}, {-707.8333433370193, -154.44224536635932}}},
{{{-708.00779269310044, -154.61669472244046}, {-701.04513225634582, -128.85970734043804}, {505.58447265625, -504.9130859375}}},

{{{164, -40}, {231.51681518554687, -40}, {279.25839233398438, 7.7416000366210938}}},
{{{279.25839233398438, 7.7416000366210938}, {275.2164306640625, 3.6996400356292725}, {271.03286743164062, -5.3290705182007514e-015}}},

{{{2.9999997378517067, 1.9737872594345709}, {2.9999997432230918, 1.9739647181863822}, {1.2414155459263587e-163, 5.2957833941332142e-315}}},
{{{2.9999047485265304, 1.9739164225694723}, {3.0000947268526112, 1.9738379076623633}, {0.61149411077591886, 0.0028382324376270418}}},

    {{{2.9999996843656502, 1.9721416019045801}, {2.9999997725237835, 1.9749798343422071},
            {5.3039068214821359e-315, 8.9546185262775165e-307}}},
    {{{2.9984791443874976, 1.974505741312242}, {2.9999992702127476, 1.9738772171479178},
            {3.0015187977319759, 1.9732495027303418}}},

    {{{0.647069409,2.97691634}, {0.946860918,3.17625612}, {1.46875407,2.65105457}}},
    {{{0,1}, {0.723699095,2.82756208}, {1.08907197,2.97497449}}},

    {{{131.37418,11414.9825}, {130.28798,11415.9328}, {130.042755,11417.4131}}},
    {{{130.585787,11418.4142}, {130.021447,11417.8498}, {130,11417}}},

    {{{130.73167037963867, 11418.546386718750}, {131.26360225677490, 11418.985778808592},
            {132, 11419 }}},
    {{{132, 11419}, {131.15012693405151, 11418.978546142578},
            {130.58578681945801, 11418.414184570313}}},

    {{{132, 11419},
            {130.73167037963867, 11418.546386718750}, {131.26360225677490, 11418.985778808592}}},
    {{{131.15012693405151, 11418.978546142578},
            {130.58578681945801, 11418.414184570313}, {132, 11419}}},

    {{{3.0774019473063863, 3.35198509346713}, {3.0757503498668397, 3.327320623945933},
            {3.0744102085015879, 3.3025879417907196}}},
    {{{3.053913680774329, 3.3310471586283938}, {3.0758730889691694, 3.3273466070370152},
            {3.0975671980059394, 3.3235031316554351}}},

    {{{3.39068129, 4.44939202}, {3.03659239, 3.81843234}, {3.06844529, 3.02100922}}},
    {{{2.10714698, 3.44196686}, {3.12180288, 3.38575704}, {3.75968569, 3.1281838}}},

    {{{2.74792918, 4.77711896}, {2.82236867, 4.23882547}, {2.82848144, 3.63729341}}},
    {{{2.62772567, 3.64823958}, {3.46652495, 3.64258364}, {4.1425079, 3.48623815}}},

    {{{1.34375, 2.03125}, {2.2734375, 2.6640625}, {3.25, 3.25}}},
    {{{3.96875, 4.65625}, {3.3359375, 3.7265625}, {2.75, 2.75}}},

    {{{0, 1}, {0.324417544, 2.27953848}, {0.664376547, 2.58940267}}},
    {{{1, 2}, {0.62109375, 2.70703125}, {0.640625, 2.546875}}},

    {{{1, 2}, {0.984375, 2.3359375}, {1.0625, 2.15625}}},
    {{{0, 1}, {0.983539095, 2.30041152}, {1.47325103, 2.61316872}}},

    {{{4.09011926, 2.20971038}, {4.74608133, 1.9335932}, {5.02469918, 2.00694987}}},
    {{{2.79472921, 1.73568666}, {3.36246373, 1.21251209}, {5, 2}}},

    {{{1.80814127, 2.41537795}, {2.23475077, 2.05922313}, {3.16529668, 1.98358763}}},
    {{{2.16505631, 2.55782454}, {2.40541285, 2.02193091}, {2.99836023, 1.68247638}}},

    {{{3, 1.875}, {3.375, 1.54296875}, {3.375, 1.421875}}},
    {{{3.375, 1.421875}, {3.3749999999999996, 1.3007812499999998}, {3, 2}}},

    {{{3.34, 8.98}, {2.83363281, 9.4265625}, {2.83796875, 9.363125}}},
    {{{2.83796875, 9.363125}, {2.84230469, 9.2996875}, {3.17875, 9.1725}}},

    {{{2.7279999999999998, 3.024}, {2.5600000000000005, 2.5600000000000005},
            {2.1520000000000001, 1.8560000000000001}}},
    {{{0.66666666666666652, 1.1481481481481481}, {1.3333333333333326, 1.3333333333333335},
            {2.6666666666666665, 2.1851851851851851}}},

    {{{2.728, 3.024}, {2.56, 2.56}, {2.152, 1.856}}},
    {{{0.666666667, 1.14814815}, {1.33333333, 1.33333333}, {2.66666667, 2.18518519}}},

    {{{0.875, 1.5}, {1.03125, 1.11022302e-16}, {1, 0}}},
    {{{0.875, 0.859375}, {1.6875, 0.73046875}, {2.5, 0.625}}},

    {{{1.64451042, 0.0942001592}, {1.53635465, 0.00152863961}, {1, 0}}},
    {{{1.27672209, 0.15}, {1.32143477, 9.25185854e-17}, {1, 0}}},

    {{{0, 0}, {0.51851851851851849, 1.0185185185185186}, {1.2592592592592591, 1.9259259259259258}}},
    {{{1.2592592592592593, 1.9259259259259265}, {0.51851851851851893, 1.0185185185185195}, {0, 0}}},

    {{{1.93281168, 2.58856757}, {2.38543691, 2.7096125}, {2.51967352, 2.34531784}}},
    {{{2.51967352, 2.34531784}, {2.65263731, 2.00639194}, {3.1212119, 1.98608967}}},
    {{{2.09544533, 2.51981963}, {2.33331524, 2.25252128}, {2.92003302, 2.39442311}}},

    {{{0.924337655, 1.94072717}, {1.25185043, 1.52836494}, {1.71793901, 1.06149951}}},
    {{{0.940798831, 1.67439357}, {1.25988251, 1.39778567}, {1.71791672, 1.06650313}}},

    {{{0.924337655, 1.94072717}, {1.39158994, 1.32418496}, {2.14967426, 0.687365435}}},
    {{{0.940798831, 1.67439357}, {1.48941875, 1.16280321}, {2.47884711, 0.60465921}}},

    {{{1.7465749139282332, 1.9930452039527999}, {1.8320006564080331, 1.859481345189089},
            {1.8731035127758437, 1.6344055934266613}}},
    {{{1.8731035127758437, 1.6344055934266613}, {1.89928170345231, 1.5006405518943067},
            {1.9223833226085514, 1.3495796165215643}}},
    {{{1.74657491, 1.9930452}, {1.87407679, 1.76762853}, {1.92238332, 1.34957962}}},
    {{{0.60797907, 1.68776977}, {1.0447864, 1.50810914}, {1.87464474, 1.63655092}}},
    {{{1.87464474, 1.63655092}, {2.70450308, 1.76499271}, {4, 3}}},

    {{{1.2071879545809394, 0.82163474041730045}, {1.1534203513372994, 0.52790870069930229},
            {1.0880000000000001, 0.29599999999999982}}},  //t=0.63155333662549329,0.80000000000000004
    {{{0.33333333333333326, 0.81481481481481488}, {0.63395173631977997, 0.68744136726313931},
            {1.205684411948591, 0.81344322326274499}}},
    {{{0.33333333333333326, 0.81481481481481488}, {0.63396444791444551, 0.68743368362444768},
            {1.205732763658403, 0.81345617746834109}}},  //t=0.33333333333333331, 0.63396444791444551
    {{{1.205684411948591, 0.81344322326274499}, {1.2057085875611198, 0.81344969999329253},
            {1.205732763658403, 0.81345617746834109}}},

    {{{1.20718795, 0.82163474}, {1.15342035, 0.527908701}, {1.088, 0.296}}},
    {{{1.20568441, 0.813443223}, {1.20570859, 0.8134497}, {1.20573276, 0.813456177}}},

    {{{41.5072916, 87.1234036}, {28.2747836, 80.9545395}, {23.5780771, 69.3344126}}},
    {{{72.9633878, 95.6593007}, {42.7738746, 88.4730382}, {31.1932785, 80.2458029}}},

    {{{31.1663962, 54.7302484}, {31.1662882, 54.7301074}, {31.1663969, 54.7302485}}},
    {{{26.0404936, 45.4260361}, {27.7887523, 33.1863051}, {40.8833242, 26.0301855}}},

    {{{29.9404074, 49.1672596}, {44.3131071, 45.3915253}, {58.1067559, 59.5061814}}},
    {{{72.6510251, 64.2972928}, {53.6989659, 60.1862397}, {35.2053722, 44.8391126}}},

    {{{52.14807018377202, 65.012420045148644}, {44.778669050208237, 66.315562705604378},
            {51.619118408823567, 63.787827046262684}}},
    {{{30.004993234763383, 93.921296668202288}, {53.384822003076991, 60.732180341802753},
            {58.652998934338584, 43.111073088306185}}},

    {{{80.897794748143198, 49.236332042718459}, {81.082078218891212, 64.066749904488631},
            {69.972305057149981, 72.968595519850993}}},
    {{{72.503745601281395, 32.952320736577882}, {88.030880716061645, 38.137194847810164},
            {73.193774825517906, 67.773492479591397}}},

    {{{67.426548091427676, 37.993772624988935}, {51.129513170665035, 57.542281234563646},
            {44.594748190899189, 65.644267382683879}}},
    {{{61.336508189019057, 82.693132843213675}, {54.825078921449354, 71.663932799212432},
            {47.727444217558926, 61.4049645128392}}},

    {{{67.4265481, 37.9937726}, {51.1295132, 57.5422812}, {44.5947482, 65.6442674}}},
    {{{61.3365082, 82.6931328}, {54.8250789, 71.6639328}, {47.7274442, 61.4049645}}},

    {{{53.774852327053594, 53.318060789841951}, {45.787877803416805, 51.393492026284981},
            {46.703936967162392, 53.06860709822206}}},
    {{{46.703936967162392, 53.06860709822206}, {47.619996130907957, 54.74372217015916},
            {53.020051653535361, 48.633140968832024}}},

    {{{50.934805397717923, 51.52391952648901}, {56.803308902971423, 44.246234610627596},
            {69.776888596721406, 40.166645096692555}}},
    {{{50.230212796400401, 38.386469101526998}, {49.855620812184917, 38.818990392153609},
            {56.356567496227363, 47.229909093319407}}},

    {{{36.148792695174222, 70.336952793070424}, {36.141613037691357, 70.711654739870085},
            {36.154708826402597, 71.088492662905836}}},
    {{{35.216235592661825, 70.580199617313212}, {36.244476835123969, 71.010897787304074},
            {37.230244263238326, 71.423156953613102}}},

    // this pair is nearly coincident, and causes the quartic code to produce bad
    // data. Mathematica doesn't think they touch. Graphically, I can't tell.
    // it may not be so bad to pretend that they don't touch, if I can detect that
    {{{369.848602, 145.680267}, {382.360413, 121.298294}, {406.207703, 121.298294}}},
    {{{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}}},

    {{{33.567436351153468, 62.336347586395924}, {35.200980274619084, 65.038561460144479},
            {36.479571811084995, 67.632178905412445}}},
    {{{41.349524945572696, 67.886658677862641}, {39.125562529359087, 67.429772735149214},
            {35.600314083992416, 66.705372160552685}}},

    {{{67.25299631583178, 21.109080184767524}, {43.617595267398613, 33.658034168577529},
            {33.38371819435676, 44.214192553988745}}},
    {{{40.476838859398541, 39.543209911285999}, {36.701186108431131, 34.8817994016458},
            {30.102144288878023, 26.739063172945315}}},

    {{{25.367434474345036, 50.4712103169743}, {17.865013304933097, 37.356741010559439},
            {16.818988838905465, 37.682915484123129}}},
    {{{16.818988838905465, 37.682915484123129}, {15.772964372877833, 38.009089957686811},
            {20.624104547604965, 41.825131596683121}}},

    {{{26.440225044088567, 79.695009812848298}, {26.085525979582247, 83.717928354134784},
            {27.075079976297072, 84.820633667838905}}},
    {{{27.075079976297072, 84.820633667838905}, {28.276546859574015, 85.988574184029034},
            {25.649263209500006, 87.166762066617025}}},

    {{{34.879150914024962, 83.862726601601125}, {35.095810134304429, 83.693473210169543},
            {35.359284111931586, 83.488069234177502}}},
    {{{54.503204203015471, 76.094098492518242}, {51.366889541918894, 71.609856061299155},
            {46.53086955445437, 69.949863036494207}}},

    {{{0, 0}, {1, 0}, {0, 3}}},
    {{{1, 0}, {0, 1}, {1, 1}}},
    {{{369.961151, 137.980698}, {383.970093, 121.298294}, {406.213287, 121.298294}}},
    {{{353.2948, 194.351074}, {353.2948, 173.767563}, {364.167572, 160.819855}}},
    {{{360.416077, 166.795715}, {370.126831, 147.872162}, {388.635406, 147.872162}}},
    {{{406.236359, 121.254936}, {409.445679, 121.254936}, {412.975952, 121.789818}}},
    {{{406.235992, 121.254936}, {425.705902, 121.254936}, {439.71994, 137.087616}}},

    {{{369.8543701171875, 145.66734313964844}, {382.36788940429688, 121.28203582763672},
            {406.21844482421875, 121.28203582763672}}},
    {{{369.96469116210938, 137.96672058105469}, {383.97555541992188, 121.28203582763672},
            {406.2218017578125, 121.28203582763672}}},

    {{{369.962311, 137.976044}, {383.971893, 121.29287}, {406.216125, 121.29287}}},

    {{{400.121704, 149.468719}, {391.949493, 161.037186}, {391.949493, 181.202423}}},
    {{{391.946747, 181.839218}, {391.946747, 155.62442}, {406.115479, 138.855438}}},
    {{{360.048828125, 229.2578125}, {360.048828125, 224.4140625}, {362.607421875, 221.3671875}}},
    {{{362.607421875, 221.3671875}, {365.166015625, 218.3203125}, {369.228515625, 218.3203125}}},
    {{{8, 8}, {10, 10}, {8, -10}}},
    {{{8, 8}, {12, 12}, {14, 4}}},
    {{{8, 8}, {9, 9}, {10, 8}}}
};

const size_t testSetCount = SK_ARRAY_COUNT(testSet);

static void oneOffTest1(skiatest::Reporter* reporter, size_t outer, size_t inner) {
    const QuadPts& q1 = testSet[outer];
    SkDQuad quad1;
    quad1.debugSet(q1.fPts);
    SkASSERT(ValidQuad(quad1));
    const QuadPts& q2 = testSet[inner];
    SkDQuad quad2;
    quad2.debugSet(q2.fPts);
    SkASSERT(ValidQuad(quad2));
    SkIntersections intersections;
    intersections.intersect(quad1, quad2);
    for (int pt = 0; pt < intersections.used(); ++pt) {
        double tt1 = intersections[0][pt];
        SkDPoint xy1 = quad1.ptAtT(tt1);
        double tt2 = intersections[1][pt];
        SkDPoint xy2 = quad2.ptAtT(tt2);
        if (!xy1.approximatelyEqual(xy2)) {
            SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                    __FUNCTION__, static_cast<int>(outer), static_cast<int>(inner),
                    tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
            REPORTER_ASSERT(reporter, 0);
        }
#if ONE_OFF_DEBUG
        SkDebugf("%s [%d][%d] t1=%1.9g (%1.9g, %1.9g) t2=%1.9g\n", __FUNCTION__,
            outer, inner, tt1, xy1.fX, xy1.fY, tt2);
#endif
    }
}

static void oneOffTests(skiatest::Reporter* reporter) {
    for (size_t outer = 0; outer < testSetCount - 1; ++outer) {
        for (size_t inner = outer + 1; inner < testSetCount; ++inner) {
            oneOffTest1(reporter, outer, inner);
        }
    }
}

static const QuadPts coincidentTestSet[] = {
    {{{4914.9990234375, 1523}, {4942.75146484375, 1523}, {4962.375, 1542.6239013671875}}},
    {{{4962.3759765625, 1542.6239013671875}, {4942.75244140625, 1523}, {4915, 1523}}},
#if 0
    {{{97.9337615966796875,100}, {88,112.94264984130859375}, {88,130}}},
    {{{88,130}, {88,124.80951690673828125}, {88.91983795166015625,120}}},
#endif
    {{{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}}},
    {{{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}}},
    {{{8, 8}, {10, 10}, {8, -10}}},
    {{{8, -10}, {10, 10}, {8, 8}}},
};

static const int coincidentTestSetCount = (int) SK_ARRAY_COUNT(coincidentTestSet);

static void coincidentTestOne(skiatest::Reporter* reporter, int test1, int test2) {
    const QuadPts& q1 = coincidentTestSet[test1];
    SkDQuad quad1;
    quad1.debugSet(q1.fPts);
    SkASSERT(ValidQuad(quad1));
    const QuadPts& q2 = coincidentTestSet[test2];
    SkDQuad quad2;
    quad2.debugSet(q2.fPts);
    SkASSERT(ValidQuad(quad2));
    SkIntersections intersections2;
    intersections2.intersect(quad1, quad2);
    REPORTER_ASSERT(reporter, intersections2.debugCoincidentUsed() >= 2);
    REPORTER_ASSERT(reporter, intersections2.used() >= 2);
    for (int pt = 0; pt < intersections2.debugCoincidentUsed(); pt += 2) {
        double tt1 = intersections2[0][pt];
        double tt2 = intersections2[1][pt];
        SkDPoint pt1 = quad1.ptAtT(tt1);
        SkDPoint pt2 = quad2.ptAtT(tt2);
        REPORTER_ASSERT(reporter, pt1.approximatelyEqual(pt2));
    }
}

static void coincidentTest(skiatest::Reporter* reporter) {
    for (int testIndex = 0; testIndex < coincidentTestSetCount - 1; testIndex += 2) {
        coincidentTestOne(reporter, testIndex, testIndex + 1);
    }
}

static void intersectionFinder(int test1, int test2) {
    const QuadPts& q1 = testSet[test1];
    const QuadPts& q2 = testSet[test2];
    SkDQuad quad1, quad2;
    quad1.debugSet(q1.fPts);
    quad2.debugSet(q2.fPts);
    double t1Seed = 0.5;
    double t2Seed = 0.8;
    double t1Step = 0.1;
    double t2Step = 0.1;
    SkDPoint t1[3], t2[3];
    bool toggle = true;
    do {
        t1[0] = quad1.ptAtT(t1Seed - t1Step);
        t1[1] = quad1.ptAtT(t1Seed);
        t1[2] = quad1.ptAtT(t1Seed + t1Step);
        t2[0] = quad2.ptAtT(t2Seed - t2Step);
        t2[1] = quad2.ptAtT(t2Seed);
        t2[2] = quad2.ptAtT(t2Seed + t2Step);
        double dist[3][3];
        dist[1][1] = t1[1].distance(t2[1]);
        int best_i = 1, best_j = 1;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (i == 1 && j == 1) {
                    continue;
                }
                dist[i][j] = t1[i].distance(t2[j]);
                if (dist[best_i][best_j] > dist[i][j]) {
                    best_i = i;
                    best_j = j;
                }
            }
        }
        if (best_i == 0) {
            t1Seed -= t1Step;
        } else if (best_i == 2) {
            t1Seed += t1Step;
        }
        if (best_j == 0) {
            t2Seed -= t2Step;
        } else if (best_j == 2) {
            t2Seed += t2Step;
        }
        if (best_i == 1 && best_j == 1) {
            if ((toggle ^= true)) {
                t1Step /= 2;
            } else {
                t2Step /= 2;
            }
        }
    } while (!t1[1].approximatelyEqual(t2[1]));
    t1Step = t2Step = 0.1;
    double t10 = t1Seed - t1Step * 2;
    double t12 = t1Seed + t1Step * 2;
    double t20 = t2Seed - t2Step * 2;
    double t22 = t2Seed + t2Step * 2;
    SkDPoint test;
    while (!approximately_zero(t1Step)) {
        test = quad1.ptAtT(t10);
        t10 += t1[1].approximatelyEqual(test) ? -t1Step : t1Step;
        t1Step /= 2;
    }
    t1Step = 0.1;
    while (!approximately_zero(t1Step)) {
        test = quad1.ptAtT(t12);
        t12 -= t1[1].approximatelyEqual(test) ? -t1Step : t1Step;
        t1Step /= 2;
    }
    while (!approximately_zero(t2Step)) {
        test = quad2.ptAtT(t20);
        t20 += t2[1].approximatelyEqual(test) ? -t2Step : t2Step;
        t2Step /= 2;
    }
    t2Step = 0.1;
    while (!approximately_zero(t2Step)) {
        test = quad2.ptAtT(t22);
        t22 -= t2[1].approximatelyEqual(test) ? -t2Step : t2Step;
        t2Step /= 2;
    }
#if ONE_OFF_DEBUG
    SkDebugf("%s t1=(%1.9g<%1.9g<%1.9g) t2=(%1.9g<%1.9g<%1.9g)\n", __FUNCTION__,
        t10, t1Seed, t12, t20, t2Seed, t22);
    SkDPoint p10 = quad1.ptAtT(t10);
    SkDPoint p1Seed = quad1.ptAtT(t1Seed);
    SkDPoint p12 = quad1.ptAtT(t12);
    SkDebugf("%s p1=(%1.9g,%1.9g)<(%1.9g,%1.9g)<(%1.9g,%1.9g)\n", __FUNCTION__,
        p10.fX, p10.fY, p1Seed.fX, p1Seed.fY, p12.fX, p12.fY);
    SkDPoint p20 = quad2.ptAtT(t20);
    SkDPoint p2Seed = quad2.ptAtT(t2Seed);
    SkDPoint p22 = quad2.ptAtT(t22);
    SkDebugf("%s p2=(%1.9g,%1.9g)<(%1.9g,%1.9g)<(%1.9g,%1.9g)\n", __FUNCTION__,
        p20.fX, p20.fY, p2Seed.fX, p2Seed.fY, p22.fX, p22.fY);
#endif
}

static void QuadraticIntersection_IntersectionFinder() {
    intersectionFinder(0, 1);
}

DEF_TEST(PathOpsQuadIntersectionOneOff, reporter) {
    oneOffTest1(reporter, 0, 1);
}

DEF_TEST(PathOpsQuadIntersectionCoincidenceOneOff, reporter) {
    coincidentTestOne(reporter, 0, 1);
}

DEF_TEST(PathOpsQuadIntersection, reporter) {
    oneOffTests(reporter);
    coincidentTest(reporter);
    standardTestCases(reporter);
    if (false) QuadraticIntersection_IntersectionFinder();
}

DEF_TEST(PathOpsQuadBinaryProfile, reporter) {
    if (!SkPathOpsDebug::gVeryVerbose) {
            return;
    }
    SkIntersections intersections;
    for (int x = 0; x < 100; ++x) {
        int outer = 0;
        int inner = outer + 1;
        do {
            const QuadPts& q1 = testSet[outer];
            SkDQuad quad1;
            quad1.debugSet(q1.fPts);
            const QuadPts& q2 = testSet[inner];
            SkDQuad quad2;
            quad2.debugSet(q2.fPts);
            (void) intersections.intersect(quad1, quad2);
            REPORTER_ASSERT(reporter, intersections.used() >= 0);  // make sure code isn't tossed
            inner += 2;
            outer += 2;
        } while (outer < (int) testSetCount);
    }
    for (int x = 0; x < 100; ++x) {
        for (size_t test = 0; test < quadraticTests_count; ++test) {
            const QuadPts& q1 = quadraticTests[test][0];
            const QuadPts& q2 = quadraticTests[test][1];
            SkDQuad quad1, quad2;
            quad1.debugSet(q1.fPts);
            quad2.debugSet(q2.fPts);
            (void) intersections.intersect(quad1, quad2);
            REPORTER_ASSERT(reporter, intersections.used() >= 0);  // make sure code isn't tossed
        }
    }
}
