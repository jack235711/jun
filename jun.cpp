#property copyright "Copyright 20017, MetaQuotes Software Corp."
#property link "https,//www.mql5.com"
#property version "1.00"
#property strict
//+------------------------------------------------------------------+
// 整列：Shift + Alt + F
// 全展開：Ctrl+K Ctrl+J
// 全畳む：Ctrl+K Ctrl+0
// 関数展開：Ctrl+K Ctrl+]
// 関数畳む：Ctrl+K Ctrl+[
// 分割：View→Edit layout
// コメントアウト：Ctrl+/
//+------------------------------------------------------------------+
//・1か月1単位で$100の利益
//逆張りを支えるMAを1分足の期間を延ばしてポジションが増えた時の上位のMAも表現する
// 変数の宣言
string Currency[10] = {"USDJPY"};
int Period[10] = {1, 5, 15, 30, 60, 240, 1440};
int BuildNumber = 0;                    // 新建オーダー数（プラス数で買い、マイナス数で売り）
int CloseNumber = 0;                    // 建閉オーダー数（プラス数で買い、マイナス数で売り）
int BuyPositionMode[10] = {};           // 買ポジションの状況（0ポジションなし/-1逆）
double BuyLots = 0;                     // 買いポジション数
double BuyProfit = 0;                   // 買いポジション利益
double BuyLowestPrice = 1000;           // 逆張買中の最小価格
double BuyLowestPriceProfit = 0;        // 逆張買中の最小価格の利益
double BuyLowestPriceTicket = 0;           // 逆張買中の最小価格のチケット番号
double MaxBuyLots = 0;                  // 最大の同時ポジション数（Print用）
int Band_Rank = -1;                     // Bandのランク
double Sigma = 0;                       // ボリンジャーバンドの分散

struct tmp_st
{
    // MACD
    double MACD1;
    double MACD2;
    // MACDのシグナル
    double Sig1;
    double Sig2;
    // MACDとシグナルの位置関係
    double MACD_Sig1;
    double MACD_Sig2;
    // MACDとMACDの関係
    double MACD_1_2;
    //　ボリンジャーバンド
    double Band_Main;
    double Band_Upper;
    double Sigma;
    double Band_Rank;
};
tmp_st st[10][10];
// トレンド判断w/MACD
void TrendMACD()
{
    // 判断材料算出
    for (int i = 0; i < 1; i++) //通貨
    {
        for (int j = 0; j < 7; j++) //時間軸
        {
            // MACD
            st[i][j].MACD1 = iMACD(Currency[i], Period[j], (1+BuyLots*BuyLots)*5, (1+BuyLots*BuyLots)*20, 3, PRICE_CLOSE, MODE_MAIN, 0);
            st[i][j].MACD2 = iMACD(Currency[i], Period[j], (1+BuyLots*BuyLots)*20, (1+BuyLots*BuyLots)*40, 3, PRICE_CLOSE, MODE_MAIN, 0);
            // シグナル（MACD移動平均線）
            st[i][j].Sig1 = iMACD(Currency[i], Period[j], (1+BuyLots*BuyLots)*5, (1+BuyLots*BuyLots)*20, 3, PRICE_CLOSE, MODE_SIGNAL, 0);
            st[i][j].Sig2 = iMACD(Currency[i], Period[j], (1+BuyLots*BuyLots)*20, (1+BuyLots*BuyLots)*40, 3, PRICE_CLOSE, MODE_SIGNAL, 0);
            // MACDとシグナル関係
            st[i][j].MACD_Sig1 = st[i][j].MACD1 - st[i][j].Sig1;
            st[i][j].MACD_Sig2 = st[i][j].MACD2 - st[i][j].Sig2;
            // MACD[1]とMACD[2]関係
            st[i][j].MACD_1_2 = st[i][j].MACD1 - st[i][j].MACD2;
            // ボリンジャー計算
            st[i][j].Band_Main = iBands(Currency[i], Period[j], 20, 0, 0, PRICE_CLOSE, MODE_MAIN, 0);
            st[i][j].Band_Upper = iBands(Currency[i], Period[j], 20, 1, 0, PRICE_CLOSE, MODE_UPPER, 0);
            st[i][j].Sigma = st[i][j].Band_Upper - st[i][j].Band_Main; 
            st[i][j].Band_Rank = (iClose("USDJPY", PERIOD_M1, 1) - st[i][j].Band_Main) / st[i][j].Sigma;
        }
    }
}

void Arrow()
{
    int x1 = MathRand();
    int x2 = MathRand();
    int x3 = MathRand();
    int y1 = MathRand();
    int y2 = MathRand();
    int y3 = MathRand();
    int z1 = x1 * y1;
    string Name1 = z1;
    int z2 = x2 * y2;
    string Name2 = z2;
    int z3 = x3 * y3;
    string Name3 = z3;
    static datetime time = Time[0];
    if (Time[0] != time)
    {
        // MACD1
        if (Band_Rank == 4)
        {
            ObjectCreate(0, Name1, OBJ_ARROW_UP, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.04);
            ObjectSetInteger(0, Name1, OBJPROP_COLOR, Yellow);
        }
        else if (Band_Rank == 3)
        {
            ObjectCreate(0, Name1, OBJ_ARROW_DOWN, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.04);
            ObjectSetInteger(0, Name1, OBJPROP_COLOR, clrAqua);
        }
        else if (Band_Rank == 2)
        {
            ObjectCreate(0, Name1, OBJ_ARROW_CHECK, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.04);
            ObjectSetInteger(0, Name1, OBJPROP_COLOR, clrGray);
        }
    }
}

// 表示
void PrintSet()
{
    Print("Band_Rank  :  ", Band_Rank);
    //Print("MaxBuyLots: ", MaxBuyLots,"    BuyLots: ", BuyLots, "       CurrentTime - BuyOpenTime:  ", CurrentTime - BuyOpenTime, "     CloseInterval:   ",CloseInterval());
}
// ポジション&利益管理
void ManageParameter()
{
    // 初期化
    BuyLots = 0;
    BuyProfit = 0;
    BuyLowestPrice = 10000;
    BuyLowestPriceProfit = 0;

    // 建玉数、利益管理
    for (int i = 0; i < OrdersTotal(); i++)
    {
        if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false)
            break;
        if (OrderType() == OP_BUY)
        {
            BuyLots += 100*OrderLots();
            BuyProfit += OrderProfit();
        }
        if(MaxBuyLots < BuyLots){MaxBuyLots = BuyLots;}
        if(BuyLowestPrice > OrderOpenPrice())
        {
            BuyLowestPrice = OrderOpenPrice();
            BuyLowestPriceTicket = OrderTicket();
            BuyLowestPriceProfit = OrderProfit();
        }
    }

    // ポジションのモード管理
    if (BuyLots == 0)
    { // ポジションなし
        BuyPositionMode[0] = 0;
    }
    else
    { // ポジションあり
        BuyPositionMode[0] = -1;
    }
}
//建玉インターバル
double OpenInterval(){
    double r = 0;
    //ボリンジャーバンド
    for(int i=0;i<8;i++){
        if(st[0][i].Band_Rank > 0){
            st[0][i].Band_Rank = 0;
        }
        r += st[0][0].Sigma * st[0][i].Band_Rank * (-1);
    }
    //MACD
    for(int j=0;j<7;j++){
        if(st[0][j].MACD_Sig1 > 0){
            r -= st[0][0].Sigma;
        }
        else if(st[0][j].MACD_Sig1 < 0){
            r += st[0][0].Sigma;
        }
        if(st[0][j].MACD_Sig2 > 0){
            r -= st[0][0].Sigma;
        }
        else if(st[0][j].MACD_Sig2 < 0){
            r += st[0][0].Sigma;
        }
    }
    //建玉数
    r += st[0][0].Sigma * BuyLots;
    if(r < st[0][0].Sigma){
        r = st[0][0].Sigma;
    }
    return r;
}
//閉玉インターバル
double CloseInterval(){
    double r = 0;
    //ボリンジャーバンド
    for(int i=0;i<7;i++){
        if(st[0][i].Band_Rank < 0){
            st[0][i].Band_Rank = 0;
        }
        r += st[0][0].Sigma * st[0][i].Band_Rank;
    }
    //MACD
    for(int j=0;j<7;j++){
        if(st[0][j].MACD_Sig1 < 0){
            r -= st[0][0].Sigma;
        }
        else if(st[0][j].MACD_Sig1 > 0){
            r += st[0][0].Sigma;
        }
        if(st[0][j].MACD_Sig2 < 0){
            r -= st[0][0].Sigma;
        }
        else if(st[0][j].MACD_Sig2 > 0){
            r += st[0][0].Sigma;
        }
    }
    if(r < st[0][0].Sigma){
        r = st[0][0].Sigma;
    }
    return r;
}
// 買建て条件・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
void BuildOrder()
{
    // ポジションスタート
    if (BuyPositionMode[0] == 0)
    {
        if (//一つ前が下足
        iOpen("USDJPY", PERIOD_M1, 1) > iClose("USDJPY", PERIOD_M1, 1)
        //ヒゲよりも実体のほうが大きい
        && (iClose("USDJPY", PERIOD_M1, 1) - iLow("USDJPY", PERIOD_M1, 1) + iHigh("USDJPY", PERIOD_M1, 1) - iOpen("USDJPY", PERIOD_M1, 1)) < (iOpen("USDJPY", PERIOD_M1, 1) - iClose("USDJPY", PERIOD_M1, 1))
        //一つ前よりも価格が低い
        && iClose("USDJPY", PERIOD_M1, 0) < iLow("USDJPY", PERIOD_M1, 1)
        //短期MACDが下方向
        && st[0][0].MACD_Sig1 < 0 && st[0][0].MACD_Sig2 < 0 
        )
        {
            BuildNumber = 1;
        }
    }
    else if (BuyPositionMode[0] == -1)
    {
        //順張チャージ照査
    
        //逆張チャージ照査
        if (//前ポジションとのインターバル確保
        iClose("USDJPY", PERIOD_M1, 0) < BuyLowestPrice - OpenInterval()
        //一つ前が下足
        && iOpen("USDJPY", PERIOD_M1, 1) > iClose("USDJPY", PERIOD_M1, 1)
        //一つ前がヒゲよりも実体のほうが大きい
        && (iClose("USDJPY", PERIOD_M1, 1) - iLow("USDJPY", PERIOD_M1, 1) + iHigh("USDJPY", PERIOD_M1, 1) - iOpen("USDJPY", PERIOD_M1, 1)) < (iOpen("USDJPY", PERIOD_M1, 1) - iClose("USDJPY", PERIOD_M1, 1))
        //一つ前よりも価格が低い
        && iClose("USDJPY", PERIOD_M1, 0) < iLow("USDJPY", PERIOD_M1, 1)
        //短期MACDが下方向
        && st[0][0].MACD_Sig1 < 0 && st[0][0].MACD_Sig2 < 0 
        )
        {
            BuildNumber = 1;
        }
    }
}

// 買閉め条件・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
void CloseOrder()
{
    if (BuyPositionMode[0] == -1)
    {
        // 全体買閉照査
        if (//インターバル以上の利益
        BuyLowestPriceProfit > CloseInterval()
        //一つ前が上足
        && iOpen("USDJPY", PERIOD_M1, 1) < iClose("USDJPY", PERIOD_M1, 1)
        //一つ前がヒゲよりも実体が大きい
        && (iOpen("USDJPY", PERIOD_M1, 1) - iLow("USDJPY", PERIOD_M1, 1) + iHigh("USDJPY", PERIOD_M1, 1) - iClose("USDJPY", PERIOD_M1, 1)) < (iClose("USDJPY", PERIOD_M1, 1) - iOpen("USDJPY", PERIOD_M1, 1))
        //一つ前よりも価格が高い
        && iClose("USDJPY", PERIOD_M1, 0) > iHigh("USDJPY", PERIOD_M1, 1)
        //短期MACDが上方向
        && st[0][0].MACD_Sig1 > 0 && st[0][0].MACD_Sig2 > 0 
        )
        {
            CloseNumber = -1;
        }
        // 単独買閉照査
        if(//長期MACDが下方向
        (st[0][5].MACD_Sig1 < 0 || st[0][6].MACD_Sig1 < 0 )
        //インターバル以上の利益
        && BuyProfit > CloseInterval()
        //一つ前が上足
        && iOpen("USDJPY", PERIOD_M1, 1) < iClose("USDJPY", PERIOD_M1, 1)
        //一つ前がヒゲよりも実体が大きい
        && (iOpen("USDJPY", PERIOD_M1, 1) - iLow("USDJPY", PERIOD_M1, 1) + iHigh("USDJPY", PERIOD_M1, 1) - iClose("USDJPY", PERIOD_M1, 1)) < (iClose("USDJPY", PERIOD_M1, 1) - iOpen("USDJPY", PERIOD_M1, 1))
        //一つ前よりも価格が高い
        && iClose("USDJPY", PERIOD_M1, 0) > iHigh("USDJPY", PERIOD_M1, 1)
        //短期MACDが上方向
        && st[0][0].MACD_Sig1 > 0 && st[0][0].MACD_Sig2 > 0 
        )
        {
            CloseNumber = -2;
        }
    }
}

// 売買実行
void TradingExecution()
{
    int Ticket = -1;
    // ロング建て
    if (BuildNumber > 0){
        for (int i = 0; i < BuildNumber; i++)
        {
            while (Ticket < 0)
            {
                Ticket = OrderSend(Currency[0], OP_BUY, 0.01, MarketInfo(Currency[0], MODE_ASK), 3, 0, 0, "Jun", 0, 0, Blue);
                if (Ticket < 0)
                {
                    Print("OrderSend failed with error #", GetLastError());
                }
            }
        }
    }
    BuildNumber = 0;
    // ロング閉じ
    if(CloseNumber == -1){
        for (int i = 0; i < OrdersTotal(); i++){
            if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false)
                break;
            else {
                bool Closed = OrderClose(OrderTicket(), OrderLots(), OrderClosePrice(), 3, clrNONE);
            }
        }
    }
    CloseNumber = 0;
}

// 以下本線---------------------------------------------------------------------
int OnInit() { return (INIT_SUCCEEDED); }
void OnDeinit() {}
void OnTick()
{
    // 下準備
    Arrow();
    ManageParameter();
    TrendMACD();
    PrintSet();

    // 条件照査
    BuildOrder();
    CloseOrder();

    // トレード執行
    TradingExecution();
}