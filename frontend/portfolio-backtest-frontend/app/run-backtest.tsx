export const handleRunBacktest = async (strategies, setResults, setShowGraph) => {
    // Simulate backtest logic here
    // This is a placeholder for actual backtest logic; replace with your API call or logic.
    const requestData = {
        portfolioName: strategies.map(s => s.portfolioName),
        startDate: strategies.map(s => s.startDate),
        endDate: strategies.map(s => s.endDate),
        startingAmount: strategies.map(s => parseFloat(s.initialAmount) || 0),
        monthlyDeposit: strategies.map(s => parseFloat(s.monthlyDeposit) || 0),
        reinvestmentPolicy: strategies.map(s => s.reinvestmentPolicy),
        recurrentInvestmentAmount: strategies.map(s => parseFloat(s.recurrentInvestmentAmount) || 0),
        rinvInvestmentNbMonthsFrequency: strategies.map(s => parseInt(s.rinvInvestmentNbMonthsFrequency) || 0),
        rinvInvestmentMonthlyWeekNum: strategies.map(s => parseInt(s.rinvInvestmentMonthlyWeekNum) || 0),
        rinvInvestmentWeekDay: strategies.map(s => parseInt(s.rinvInvestmentWeekDay) || 0),
        rinvRebalancingThreshold: strategies.map(s => parseFloat(s.rinvRebalancingThreshold) || 0),
        rinvRebalancingFreqMinNbDays: strategies.map(s => parseInt(s.rinvRebalancingFreqMinNbDays) || 0),
        rinvAllocations: strategies.map(s => {
          const formattedAssets = s.assets.reduce((acc, asset) => {
            if (asset.symbol && asset.allocation) {
              acc[asset.symbol] = parseFloat(asset.allocation);
            }
            return acc;
          }, {});
          return [`${JSON.stringify(formattedAssets)}`.replace(/"/g, '').replace('{', '').replace('}', '')];
        }),
        assets: strategies.map(s => 
          s.assets.filter(asset => asset.symbol && asset.allocation).map(asset => asset.symbol)
        )
      };
  
      try {
        const response = await fetch("http://localhost:8080/run-backtest", {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify(requestData)
        });
  
        const rawText = await response.text();
        if (!response.ok) {
          throw new Error(`HTTP Error ${response.status}: ${rawText}`);
        }
  
        const data = JSON.parse(rawText);
        setResults(data);
        setShowGraph(true);
      } catch (error) {
        console.error("Error running backtest:", error);
      }
  };