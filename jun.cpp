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
// 上昇：2022/8/12(133.4)~2022/10/14(148.7)
// 下降：2022/10/14(148.7)~2023/1/13(127.8)
// V：2022/12/16(136.7)~2023/1/13(127.8)~2023/2/24(136.4)
// 逆V：2022/8/12(133.4)~2022/10/14(148.7)~2023/1/13(127.8)
//+------------------------------------------------------------------+
// 1年	31556926秒
// 1ヶ月	2629743秒
// 1週間	604800秒
// 1日	86400秒
// 1時間	3600秒
//+------------------------------------------------------------------+
// トレンド：長期順張り環境
// 棘：売買ポイント←要改良
// 難平インターバル：短期逆張り建て
// 積立インターバル：短期順張り建て
// 玉閉インターバル：←要戦略
//+------------------------------------------------------------------+
// 変数の宣言
string Currency[10] = {"USDJPY"};
int Period[10] = {1, 5, 15, 30, 60, 240, 1440, 10080, 43200};
int BuildNumber = 0;                    // 新建オーダー数（プラス数で買い、マイナス数で売り）
int CloseNumber = 0;                    // 建閉オーダー数（プラス数で買い、マイナス数で売り）
int BuyPositionMode[10] = {};           // 買ポジションの状況（0ポジションなし/-1逆）
double BuyLots = 0;                     // 買いポジション数
double BuyProfit = 0;                   // 買いポジション利益
double BuyAverage = 0;                  // 買いポジション平均価格
datetime BuyOpenTime = 0;               // 買いポジションの時刻
double BuyLowestPrice = 1000;           // 逆張買中の最小価格
double BuyHighestPrice = 0;             // 逆張買中の最大価格
double BuyLowestPriceProfit = 0;        // 逆張買中の最小価格の利益
double BuyLowestPriceTicket = 0;        // 逆張買中の最小価格のチケット番号
double MaxBuyLots = 0;                  // 最大の同時ポジション数（Print用）
double Band_Rank = -1;                  // Bandのランク
double Sigma = 1;                       // ボリンジャーバンドの分散


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
            st[i][j].Band_Rank = (MarketInfo("USDJPY",MODE_BID)- st[i][j].Band_Main) / st[i][j].Sigma;
        }
    }
}
// 矢印
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
        if(ValueTrendAdd() == 1){
            ObjectCreate(0, Name1, OBJ_ARROW_UP, 0, Time[0], iClose("USDJPY", PERIOD_M1, 0));
            ObjectSetInteger(0, Name1, OBJPROP_COLOR, Red);
        }
    }
    if (Time[0] != time)
    {
        if(ValueTrendClose() == 1){
            ObjectCreate(0, Name2, OBJ_ARROW_DOWN, 0, Time[0], iClose("USDJPY", PERIOD_M1, 0));
            ObjectSetInteger(0, Name2, OBJPROP_COLOR, Aqua);
        }
    }
}
// 表示
void PrintSet()
{ 
    Print("MaxBuyLots: ", MaxBuyLots,"    BuyLots: ", BuyLots,  "     CloseInterval:   ",CloseInterval(), "     OpenInterval:   ",OpenInterval());
    
}
// ポジション&利益管理
void ManageParameter()
{
    // 初期化
    BuyLots = 0;
    BuyProfit = 0;
    BuyLowestPrice = 10000;
    BuyHighestPrice = 0;
    BuyLowestPriceProfit = 0;
    BuyAverage = 0;

    // 建玉数、利益管理
    for (int i = 0; i < OrdersTotal(); i++)
    {
        if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false)
            break;
        if (OrderType() == OP_BUY)
        {
            BuyLots += 100*OrderLots();
            BuyProfit += MarketInfo("USDJPY",MODE_BID) - OrderOpenPrice();
            BuyAverage += OrderOpenPrice();
        }
        if(MaxBuyLots < BuyLots){MaxBuyLots = BuyLots;}
        if(BuyLowestPrice > OrderOpenPrice())
        {
            BuyLowestPrice = OrderOpenPrice();
            BuyLowestPriceTicket = OrderTicket();
            BuyLowestPriceProfit = MarketInfo("USDJPY",MODE_BID) - OrderOpenPrice();
        }
        if(BuyHighestPrice < OrderOpenPrice())
        {
            BuyHighestPrice = OrderOpenPrice();
        }
    }
    if(BuyLots != 0){
        BuyAverage /= BuyLots;
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
//棘判断（新建）
double ValueTrendOpen(){
    double a = 0;
    //直近過去に現在値以上の高値が存在している
    for(int i = 3; i < 10 + MathPow(10, -st[0][0].Band_Rank); i++){
        if(iLow("USDJPY", PERIOD_M1, 2) >= iLow("USDJPY", PERIOD_M1, i)){
            a = 1;
        }
    }
    if(a == 0 
    && (iOpen("USDJPY", PERIOD_M1, 1) + iClose("USDJPY", PERIOD_M1, 1))/2 > iClose("USDJPY", PERIOD_M1, 0)
    && iLow("USDJPY", PERIOD_M1, 0) > iLow("USDJPY", PERIOD_M1, 1)
    && st[0][0].Band_Rank < -1){
        return 1;
    }else{
        return 0;
    }
}
//棘判断（追建）
double ValueTrendAdd(){
    double b=0;
    //直近過去に現在値以上の高値が存在している
    for(int i = 2; i < 10 + MathPow(10,-st[0][0].Band_Rank); i++){
        if(iLow("USDJPY", PERIOD_M1, 1) >= iLow("USDJPY", PERIOD_M1, i)){
            b = 1;
        }
    }
    if(b == 0 
    && (iOpen("USDJPY", PERIOD_M1, 1) + iClose("USDJPY", PERIOD_M1, 1))/2 > iClose("USDJPY", PERIOD_M1, 0)
    && iLow("USDJPY", PERIOD_M1, 0) > iLow("USDJPY", PERIOD_M1, 1)
    && st[0][0].Band_Rank < -1){
        return 1;
    }else{
        return 0;
    }
}
//棘判断（玉閉）
double ValueTrendClose(){
    double c=0;
    //直近過去に現在値以上の高値が存在している
    for(int i = 3; i < 100 + MathPow(10,st[0][0].Band_Rank); i++){
        if(iHigh("USDJPY", PERIOD_M1, 2) <= iHigh("USDJPY", PERIOD_M1, i)){
            c = 1;
        }
    }
    if(c == 0 
    && (iOpen("USDJPY", PERIOD_M1, 2) + iClose("USDJPY", PERIOD_M1, 2))/2 < iClose("USDJPY", PERIOD_M1, 0)
    && iHigh("USDJPY", PERIOD_M1, 1) < iHigh("USDJPY", PERIOD_M1, 2)
    && st[0][0].Band_Rank > 1){
        return 1;
    }else{
        return 0;
    }
}
//建玉難平インターバル
double OpenInterval(){
    double r = 0;
    //ボリンジャーバンド&MACD
    for(int i=0;i<9;i++){
        if(st[0][i].Band_Rank < 0){
            if(st[0][i].MACD_Sig1 < 0){
                r += st[0][i].Sigma * st[0][i].Band_Rank * (-1);
            }
            if(st[0][i].MACD_Sig2 < 0){
                r += st[0][i].Sigma * st[0][i].Band_Rank * (-1);
            }
        }
    }
    //建玉数
    r += st[0][0].Sigma * BuyLots;
    return r;
}
//建玉積立インターバル
datetime OpenTimeInterval(){
    double r = 0;
    //ボリンジャーバンド&MACD
    for(int i=0;i<9;i++){
        if(st[0][i].Band_Rank < 0){
            if(st[0][i].MACD_Sig1 < 0){
                r += st[0][i].Sigma * st[0][i].Band_Rank * (-1);
            }
            if(st[0][i].MACD_Sig2 < 0){
                r += st[0][i].Sigma * st[0][i].Band_Rank * (-1);
            }
        }
    }
    return 3600*(BuyLots + r);
}
//閉玉難平インターバル
double CloseInterval(){
    double r = 0;
    //ボリンジャーバンド&MACD
    for(int i=0;i<5;i++){
        if(st[0][i].Band_Rank > 0){
            if(st[0][i].MACD_Sig1 > 0){
                r += st[0][i].Sigma * st[0][i].Band_Rank;
            }
            if(st[0][i].MACD_Sig2 > 0){
                r += st[0][i].Sigma * st[0][i].Band_Rank;
            }
        }
    }
    return r;
}
// 買建て条件・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
    void BuildOrder()   
{
    // ポジションスタート
    if (BuyPositionMode[0] == 0)
    {
        if (ValueTrendOpen() == 1)
        {
            BuildNumber = 1;
        }
    }
    else if (BuyPositionMode[0] == -1)
    {
        //逆張チャージ照査　難平価格インターバル確保
        if (ValueTrendAdd() == 1
        && MarketInfo("USDJPY",MODE_ASK) < BuyLowestPrice - OpenInterval())
        {
            BuildNumber = 1;
        }
        //順張チャージ照査　積立時間インターバル確保
        else if(ValueTrendAdd() == 1
        && TimeCurrent() > BuyOpenTime + OpenTimeInterval()
        && MarketInfo("USDJPY",MODE_BID) > BuyHighestPrice)
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
        if (//トレンド判断
        ValueTrendClose() == 1
        && BuyLots > 1
        //とりあえず+の利益
        && BuyProfit > 0.1
        )
        {
            CloseNumber = -1;
        }
        // 単独買閉照査
        else if(//トレンド判断
        ValueTrendClose() == 1
        //インターバル以上の利益
        && BuyLowestPriceProfit > CloseInterval()
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
        BuyOpenTime = TimeCurrent();
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
    else if(CloseNumber == -2){
        for (int i = 0; i < OrdersTotal(); i++){
            if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false)
                break;
            else if(OrderTicket() == BuyLowestPriceTicket){
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
    TrendMACD();
    ManageParameter();
    PrintSet();
    Arrow();

    // 条件照査
    BuildOrder();
    CloseOrder();

    // トレード執行
    TradingExecution();
}