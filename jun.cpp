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
double BuyContPrice = 1000;             // 逆張買中の最小価格
double BuyContPriceProfit = 0;          // 逆張買中の最小価格の利益
int BuyContPriceTicket = 0;             // 逆張買中の最小価格のチケット番号
double ProfitPot = 0;                       // 最小ポジション取引の累積利益
int BuyPositionMode[10] = {};           // 買ポジションの状況（0ポジションなし/-1逆）
double BuyLots = 0;                     // 買いポジション数
double BuyProfit = 0;                   // 買いポジション利益
double MaxBuyLots = 0;                  // 最大の同時ポジション数（Print用）
double TripleTop = 0;                   // 三尊天井
double TripleBottom = 0;                // 逆三尊
int MACD_Score[10] = {};                     // MACDのスコア:-7~7

struct tmp_st
{
    // MACD
    double MACD1[20];
    double MACD2[20];
    // MACDのシグナル
    double Sig1[20];
    double Sig2[20];
    // MACDとシグナルの位置関係
    double MACD_Sig1[20];
    double MACD_Sig2[20];
    // MACDの傾き
    double MACD_Trend1[20];
    double MACD_Trend2[20];
    // MACDとシグナルの位置関係の傾き
    double MACD_SigTrend1[20];
    double MACD_SigTrend2[20];
    // トレンドの長さ
    int TrendLength;
};
tmp_st st[10][10];
// トレンド判断w/MACD
void TrendMACD()
{
    // 判断材料算出
    for (int i = 0; i < 7; i++) //通貨
    {
        for (int j = 0; j < 8; j++) //時間軸
        {
            for (int k = 0; k < 19; k++) //バー
            {
                // MACD
                st[i][j].MACD1[k] = iMACD(Currency[i], Period[j], 5, 20, 3, PRICE_CLOSE, 0, k);
                st[i][j].MACD2[k] = iMACD(Currency[i], Period[j], 20, 40, 3, PRICE_CLOSE, 0, k);
                // シグナル（MACD移動平均線）
                st[i][j].Sig1[k] = iMACD(Currency[i], Period[j], 5, 20, 3, PRICE_CLOSE, 1, k);
                st[i][j].Sig2[k] = iMACD(Currency[i], Period[j], 20, 40, 3, PRICE_CLOSE, 1, k);
                // MACDとシグナル関係
                st[i][j].MACD_Sig1[k] = st[i][j].MACD1[k] - st[i][j].Sig1[k];
                st[i][j].MACD_Sig2[k] = st[i][j].MACD2[k] - st[i][j].Sig2[k];
                // MACDの傾き
                st[i][j].MACD_Trend1[k] = st[i][j].MACD1[k] - st[i][j].MACD1[k + 1];
                st[i][j].MACD_Trend2[k] = st[i][j].MACD2[k] - st[i][j].MACD2[k + 1];
                // MACDとシグナル関係の傾き
                st[i][j].MACD_SigTrend1[k] = st[i][j].MACD_Sig1[k] - st[i][j].MACD_Sig1[k + 1];
                st[i][j].MACD_SigTrend2[k] = st[i][j].MACD_Sig2[k] - st[i][j].MACD_Sig2[k + 1];
            }
        }
    }
    //MACD
    for(int i=0;i<10;i++){
        MACD_Score[i] = 0;
    }
    for(int i=0;i<7;i++){
        if(st[0][i].MACD_Sig1[0] > 0){    
            MACD_Score[i]++;
        }
        else if(st[0][i].MACD_Sig1[0] < 0){    
            MACD_Score[i]--;
        }
        if(st[0][i].MACD_Sig2[0] > 0){    
            MACD_Score[i]++;
        }
        else if(st[0][i].MACD_Sig2[0] < 0){
            MACD_Score[i]--;
        }
    }
}
//三尊判定
void TripleMountain()
{
    if(iHigh("USDJPY", PERIOD_M1, 0) > iHigh("USDJPY", PERIOD_M1, 1)
    && iLow("USDJPY", PERIOD_M1, 0) > iLow("USDJPY", PERIOD_M1, 1)
    && iHigh("USDJPY", PERIOD_M1, 1) > iHigh("USDJPY", PERIOD_M1, 2)
    && iLow("USDJPY", PERIOD_M1, 1) > iLow("USDJPY", PERIOD_M1, 2)
    && iHigh("USDJPY", PERIOD_M1, 2) < iHigh("USDJPY", PERIOD_M1, 3)
    && iLow("USDJPY", PERIOD_M1, 2) < iLow("USDJPY", PERIOD_M1, 3)
    && iHigh("USDJPY", PERIOD_M1, 3) < iHigh("USDJPY", PERIOD_M1, 4)
    && iLow("USDJPY", PERIOD_M1, 3) < iLow("USDJPY", PERIOD_M1, 4))
    {
        TripleBottom = iLow("USDJPY", PERIOD_M1, 2);
    }
    if(iHigh("USDJPY", PERIOD_M1, 0) < iHigh("USDJPY", PERIOD_M1, 1)
    && iLow("USDJPY", PERIOD_M1, 0) < iLow("USDJPY", PERIOD_M1, 1)
    && iHigh("USDJPY", PERIOD_M1, 1) < iHigh("USDJPY", PERIOD_M1, 2)
    && iLow("USDJPY", PERIOD_M1, 1) < iLow("USDJPY", PERIOD_M1, 2)
    && iHigh("USDJPY", PERIOD_M1, 2) > iHigh("USDJPY", PERIOD_M1, 3)
    && iLow("USDJPY", PERIOD_M1, 2) > iLow("USDJPY", PERIOD_M1, 3)
    && iHigh("USDJPY", PERIOD_M1, 3) > iHigh("USDJPY", PERIOD_M1, 4)
    && iLow("USDJPY", PERIOD_M1, 3) > iLow("USDJPY", PERIOD_M1, 4))
    {
        TripleTop = iHigh("USDJPY", PERIOD_M1, 2);
    }
}

// 矢印判定
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
        if (st[0][0].MACD_Sig1[0] > 0 && st[0][0].MACD_Sig2[0] > 0)
        {
            ObjectCreate(0, Name1, OBJ_ARROW_UP, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.04);
            ObjectSetInteger(0, Name1, OBJPROP_COLOR, Yellow);
        }
        else if (st[0][0].MACD_Sig1[0] < 0 && st[0][0].MACD_Sig2[0] < 0)
        {
            ObjectCreate(0, Name1, OBJ_ARROW_DOWN, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.04);
            ObjectSetInteger(0, Name1, OBJPROP_COLOR, clrAqua);
        }
        else
        {
            ObjectCreate(0, Name1, OBJ_ARROW_CHECK, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.04);
            ObjectSetInteger(0, Name1, OBJPROP_COLOR, clrGray);
        }
        // MACD2
        if (st[0][0].MACD_Sig1[0] > 0 && st[0][0].MACD_Sig2[0] > 0)
        {
            ObjectCreate(0, Name2, OBJ_ARROW_UP, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.02);
            ObjectSetInteger(0, Name2, OBJPROP_COLOR, Yellow);
        }
        else if (st[0][0].MACD_Sig1[0] < 0 && st[0][0].MACD_Sig2[0] < 0)
        {
            ObjectCreate(0, Name2, OBJ_ARROW_DOWN, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.02);
            ObjectSetInteger(0, Name2, OBJPROP_COLOR, clrAqua);
        }
        else
        {
            ObjectCreate(0, Name2, OBJ_ARROW_CHECK, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1) - 0.02);
            ObjectSetInteger(0, Name2, OBJPROP_COLOR, clrGray);
        }
        // MACD3
        if (st[0][2].MACD_Sig1[0] > 0 && st[0][2].MACD_Sig2[0] > 0)
        {
            ObjectCreate(0, Name3, OBJ_ARROW_UP, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1));
            ObjectSetInteger(0, Name3, OBJPROP_COLOR, Yellow);
        }
        else if (st[0][2].MACD_Sig1[0] < 0 && st[0][2].MACD_Sig2[0] < 0)
        {
            ObjectCreate(0, Name3, OBJ_ARROW_DOWN, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1));
            ObjectSetInteger(0, Name3, OBJPROP_COLOR, clrAqua);
        }
        else
        {
            ObjectCreate(0, Name3, OBJ_ARROW_CHECK, 0, Time[0], iMA("USDJPY", "PERIOD_M1", 40, 0, MODE_EMA, PRICE_CLOSE, 1));
            ObjectSetInteger(0, Name3, OBJPROP_COLOR, clrGray);
        }
        time = Time[0];
    }
}
// 表示
void PrintSet()
{
    Print("MaxBuyLots: ", MaxBuyLots,"    BuyLots: ", BuyLots,"    PositionInterval: ",NormalizeDouble(OpenInterval(), 2),"    NowInterval: ", NormalizeDouble(BuyContPrice - iClose("USDJPY", PERIOD_M1, 0), 2));
}
// ポジション&利益管理
void ManageParameter()
{
    // 初期化
    BuyLots = 0;
    BuyProfit = 0;
    BuyContPrice = 10000;

    // 建玉数、利益管理
    for (int i = 0; i < OrdersTotal(); i++)
    {
        if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false)
            break;
        if (OrderType() == OP_BUY)
        {
            BuyLots += OrderLots();
            BuyProfit += OrderProfit();
        }
        if(MaxBuyLots < BuyLots){MaxBuyLots = BuyLots;}
        if(BuyContPrice > OrderOpenPrice())
        {
            BuyContPrice = OrderOpenPrice();
            BuyContPriceTicket = OrderTicket();
            BuyContPriceProfit = OrderProfit();
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
//逆張間隔判断
double OpenInterval(){
    double r = NormalizeDouble(1000 + BuyLots*2000, 3) * MarketInfo("USDJPY", MODE_TICKSIZE);
    return r;
}
//決済利益判断
double CloseInterval(){
    double r = NormalizeDouble(2000 + BuyLots*1000, 3) * MarketInfo("USDJPY", MODE_TICKSIZE);
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
        && iClose("USDJPY", PERIOD_M1, 0) < iClose("USDJPY", PERIOD_M1, 1)
        //MACDが上方向
        //&& MACD_Score < 0
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
        iClose("USDJPY", PERIOD_M1, 0) < BuyContPrice - OpenInterval()
        //一つ前が下足
        && iOpen("USDJPY", PERIOD_M1, 1) > iClose("USDJPY", PERIOD_M1, 1)
        //一つ前がヒゲよりも実体のほうが大きい
        && (iClose("USDJPY", PERIOD_M1, 1) - iLow("USDJPY", PERIOD_M1, 1) + iHigh("USDJPY", PERIOD_M1, 1) - iOpen("USDJPY", PERIOD_M1, 1)) < (iOpen("USDJPY", PERIOD_M1, 1) - iClose("USDJPY", PERIOD_M1, 1))
        //一つ前よりも価格が低い
        && iClose("USDJPY", PERIOD_M1, 0) < iClose("USDJPY", PERIOD_M1, 1)
        //MACDが上方向
        //&& MACD_Score < 0
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
        // 最小買閉照査
        if (//インターバル以上の利益
        BuyContPriceProfit > CloseInterval()
        //一つ前が上足
        && iOpen("USDJPY", PERIOD_M1, 1) < iClose("USDJPY", PERIOD_M1, 1)
        //一つ前がヒゲよりも実体が大きい
        && (iOpen("USDJPY", PERIOD_M1, 1) - iLow("USDJPY", PERIOD_M1, 1) + iHigh("USDJPY", PERIOD_M1, 1) - iClose("USDJPY", PERIOD_M1, 1)) < (iClose("USDJPY", PERIOD_M1, 1) - iOpen("USDJPY", PERIOD_M1, 1))
        //一つ前よりも価格が高い
        && iClose("USDJPY", PERIOD_M1, 0) > iClose("USDJPY", PERIOD_M1, 1)
        //MACDが下方向
        //&& MACD_Score > 0
        )
        {
            CloseNumber = -1;
            ProfitPot += BuyContPriceProfit;
        }
        // 全買閉照査
        if (//インターバル以上の利益
        BuyProfit + ProfitPot > 2*CloseInterval()
        //一つ前が上足
        && iOpen("USDJPY", PERIOD_M1, 1) < iClose("USDJPY", PERIOD_M1, 1)
        //一つ前がヒゲよりも実体が大きい
        && (iOpen("USDJPY", PERIOD_M1, 1) - iLow("USDJPY", PERIOD_M1, 1) + iHigh("USDJPY", PERIOD_M1, 1) - iClose("USDJPY", PERIOD_M1, 1)) < (iClose("USDJPY", PERIOD_M1, 1) - iOpen("USDJPY", PERIOD_M1, 1))
        //一つ前よりも価格が高い
        && iClose("USDJPY", PERIOD_M1, 0) > iClose("USDJPY", PERIOD_M1, 1)
        //MACDが下方向
        //&& MACD_Score > 0
        )
        {
            CloseNumber = -2;
            ProfitPot = 0;
        }
    }
}

// 売買実行
void TradingExecution()
{
    int Ticket = -1;
    // ロング建て
    if (BuildNumber > 0)
    {
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
    // 最小ロング閉じ
    if(CloseNumber == -1){
        OrderSelect(BuyContPriceTicket, SELECT_BY_TICKET, MODE_TRADES);
        bool Closed = OrderClose(OrderTicket(), 1, OrderClosePrice(), 3, clrNONE);
    }
    // 全ロング閉じ
    if(CloseNumber == -2){
        OrderSelect(BuyContPriceTicket, SELECT_BY_TICKET, MODE_TRADES);
        bool Closed = OrderClose(OrderTicket(), OrderLots(), OrderClosePrice(), 3, clrNONE);
    }
    CloseNumber = 0;
}

// 以下本線---------------------------------------------------------------------
int OnInit() { return (INIT_SUCCEEDED); }
void OnDeinit() {}
void OnTick()
{
    // 下準備
    // Arrow();
    TrendMACD();
    TripleMountain();
    ManageParameter();
    PrintSet();

    // 条件照査
    BuildOrder();
    CloseOrder();

    // トレード執行
    TradingExecution();
}
