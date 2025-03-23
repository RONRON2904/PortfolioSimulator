"use client";

import React, { useState, useEffect } from "react";
import { Card, CardContent } from "@/components/ui/card";
import { Tabs, TabsList, TabsTrigger, TabsContent } from "@/components/ui/tabs";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Plus, Play, PlusCircle } from "lucide-react";
import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid, Brush, Legend } from "recharts";

export default function PortfolioConfig() {
  // Initial empty strategy template
  const emptyStrategy = {
    portfolioName: "",
    startDate: "",
    endDate: "",
    initialAmount: "",
    monthlyDeposit: "",
    reinvestmentPolicy: "",
    recurrentInvestmentAmount: "",
    rinvInvestmentNbMonthsFrequency: "",
    rinvInvestmentMonthlyWeekNum: "",
    rinvInvestmentWeekDay: "",
    rinvRebalancingThreshold: "",
    rinvRebalancingFreqMinNbDays: "",
    assets: Array(5).fill({ symbol: "", allocation: "" })
  };

  // Now we manage an array of strategies
  const [strategies, setStrategies] = useState([{ ...emptyStrategy }]);

  const mockData = [
  { time: "2025-01-01", portfolio1: 500, portfolio2: 500 },
  { time: "2025-02-01", portfolio1: 1500, portfolio2: 300 },
  { time: "2025-03-01", portfolio1: 1000, portfolio2: 1000 },
  { time: "2025-04-01", portfolio1: 1800, portfolio2: 2500 },
  { time: "2025-05-01", portfolio1: 1400, portfolio2: 1000 },
  { time: "2025-06-01", portfolio1: 2500, portfolio2: 3500 },
  ];

  const [availableSymbols, setAvailableSymbols] = useState([]);
  const [results, setResults] = useState(mockData);
  const [showGraph, setShowGraph] = useState(true);
  const [activeTab, setActiveTab] = useState("settings");


  const portfolioKeys = Object.keys(results[0]).filter((key) => key !== "time");
  
  useEffect(() => {
    // Simulate fetching available symbols from a file
    setAvailableSymbols(["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "CSSPX.MI", "IDUS.L", "EGLN.L", "HPQ", "TTE.PA", "WMT", "NVDA"]);
  }, []);

  const addNewStrategy = () => {
    setStrategies([...strategies, { ...emptyStrategy }]);
  };
  
  const updateSettings = (strategyIndex, field, value) => {
    setStrategies(prevStrategies => {
      const updatedStrategies = [...prevStrategies];
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        [field]: value
      };
      return updatedStrategies;
    });
  };

  const updateAsset = (strategyIndex, assetIndex, field, value) => {
    setStrategies(prevStrategies => {
      const updatedStrategies = [...prevStrategies];
      const updatedAssets = [...updatedStrategies[strategyIndex].assets];
      updatedAssets[assetIndex] = { ...updatedAssets[assetIndex], [field]: value };
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        assets: updatedAssets
      };
      return updatedStrategies;
    });
  };

  const addAsset = (strategyIndex) => {
    setStrategies(prevStrategies => {
      const updatedStrategies = [...prevStrategies];
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        assets: [...updatedStrategies[strategyIndex].assets, { symbol: "", allocation: "" }]
      };
      return updatedStrategies;
    });
  };
  
  const handleRunBacktest = async () => {
    // Create arrays for each field with values from all strategies
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

    console.log("REQUESTED DATA");
    console.log(JSON.stringify(requestData));
    
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
      console.log("Raw response from backend:", rawText);

      const data = JSON.parse(rawText);
      console.log("Parsed JSON:", data);
      setResults(data);
      setShowGraph(true);
    } catch (error) {
      console.error("Error running backtest:", error);
    }
  };

  return (
    <div className="p-6 max-w-6xl mx-auto">
      <Card className="shadow-lg">
        <CardContent className="p-6 space-y-6">
          <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
            <div className="flex justify-between items-center mb-4">
              <TabsList className="flex gap-2">
                <TabsTrigger value="settings">Settings</TabsTrigger>
                <TabsTrigger value="portfolio-assets">Portfolio Assets</TabsTrigger>
              </TabsList>
              <Button 
                onClick={addNewStrategy} 
                className="flex items-center gap-1 bg-blue-600 hover:bg-blue-700 text-white"
              >
                <PlusCircle size={18} />
                <span>Add Strategy</span>
              </Button>
            </div>

            <TabsContent value="settings" className="space-y-4">
              <div className="border rounded-lg p-4">
                {strategies.map((strategy, strategyIndex) => (
                  <div key={strategyIndex} className="mb-6">
                    <div className="bg-gray-100 p-2 rounded-t-lg border-b">
                      <h3 className="font-semibold text-center">
                        Portfolio {strategyIndex + 1}
                      </h3>
                    </div>
                    
                    <div className="p-4 border-b border-x rounded-b-lg mb-4">
                      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Portfolio Name:</span>
                          <Input 
                            type="text" 
                            value={strategy.portfolioName} 
                            onChange={(e) => updateSettings(strategyIndex, "portfolioName", e.target.value)} 
                          />
                        </div>
                        
                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Start Date:</span>
                          <Input 
                            type="date" 
                            value={strategy.startDate} 
                            onChange={(e) => updateSettings(strategyIndex, "startDate", e.target.value)} 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">End Date:</span>
                          <Input 
                            type="date" 
                            value={strategy.endDate} 
                            onChange={(e) => updateSettings(strategyIndex, "endDate", e.target.value)} 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Initial Amount ($):</span>
                          <Input 
                            type="number" 
                            value={strategy.initialAmount} 
                            onChange={(e) => updateSettings(strategyIndex, "initialAmount", e.target.value)} 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Monthly Deposit ($):</span>
                          <Input 
                            type="number" 
                            value={strategy.monthlyDeposit} 
                            onChange={(e) => updateSettings(strategyIndex, "monthlyDeposit", e.target.value)} 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Reinvest Dividends?:</span>
                          <Input 
                            type="text" 
                            value={strategy.reinvestmentPolicy} 
                            onChange={(e) => updateSettings(strategyIndex, "reinvestmentPolicy", e.target.value)} 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Recurrent Investment Amount ($):</span>
                          <Input 
                            type="number" 
                            value={strategy.recurrentInvestmentAmount} 
                            onChange={(e) => updateSettings(strategyIndex, "recurrentInvestmentAmount", e.target.value)} 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Invest every x months?:</span>
                          <Input 
                            type="number" 
                            value={strategy.rinvInvestmentNbMonthsFrequency} 
                            onChange={(e) => updateSettings(strategyIndex, "rinvInvestmentNbMonthsFrequency", e.target.value)} 
                            placeholder="Enter x > 0" 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Week number in month:</span>
                          <Input 
                            type="number" 
                            value={strategy.rinvInvestmentMonthlyWeekNum} 
                            onChange={(e) => updateSettings(strategyIndex, "rinvInvestmentMonthlyWeekNum", e.target.value)} 
                            placeholder="0-4" 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Week day:</span>
                          <Input 
                            type="number" 
                            value={strategy.rinvInvestmentWeekDay} 
                            onChange={(e) => updateSettings(strategyIndex, "rinvInvestmentWeekDay", e.target.value)} 
                            placeholder="1-5" 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Rebalancing threshold (%):</span>
                          <Input 
                            type="number" 
                            value={strategy.rinvRebalancingThreshold} 
                            onChange={(e) => updateSettings(strategyIndex, "rinvRebalancingThreshold", e.target.value)} 
                            placeholder="Ex: 0.05" 
                          />
                        </div>

                        <div className="flex flex-col">
                          <span className="text-gray-600 text-sm mb-1">Rebalance after x days:</span>
                          <Input 
                            type="number" 
                            value={strategy.rinvRebalancingFreqMinNbDays} 
                            onChange={(e) => updateSettings(strategyIndex, "rinvRebalancingFreqMinNbDays", e.target.value)} 
                            placeholder="Enter x > 0" 
                          />
                        </div>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            </TabsContent>

            <TabsContent value="portfolio-assets" className="space-y-4">
              <div className="border rounded-lg p-4">
                {strategies.map((strategy, strategyIndex) => (
                  <div key={strategyIndex} className="mb-6">
                    <div className="bg-gray-100 p-2 rounded-t-lg border-b">
                      <h3 className="font-semibold text-center">
                        {strategy.portfolioName || `Portfolio ${strategyIndex + 1}`}
                      </h3>
                    </div>
                    
                    <div className="p-4 border-b border-x rounded-b-lg mb-4">
                      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                        {strategy.assets.map((asset, assetIndex) => (
                          <div key={assetIndex} className="space-y-2">
                            <Input
                              list={`tickers-${strategyIndex}`}
                              value={asset.symbol}
                              onChange={(e) => updateAsset(strategyIndex, assetIndex, "symbol", e.target.value)}
                              placeholder="Select or type ticker..."
                            />
                            
                            <datalist id={`tickers-${strategyIndex}`}>
                              {availableSymbols.map((symbol) => (
                                <option key={symbol} value={symbol} />
                              ))}
                            </datalist>

                            <Input 
                              type="number" 
                              value={asset.allocation} 
                              onChange={(e) => updateAsset(strategyIndex, assetIndex, "allocation", e.target.value)} 
                              placeholder="Asset Allocation (%)" 
                            />
                          </div>
                        ))}
                      </div>
                      <Button 
                        onClick={() => addAsset(strategyIndex)} 
                        className="w-full flex items-center justify-center gap-2 mt-4"
                      >
                        <Plus size={18} /> Add Asset
                      </Button>
                    </div>
                  </div>
                ))}
              </div>
              
              <div className="flex justify-center mt-6">
                <Button 
                  onClick={handleRunBacktest} 
                  className="bg-blue-600 hover:bg-blue-700 text-white flex items-center gap-2 px-6 py-2"
                >
                  <Play size={18} />
                  <span>Run Backtest</span>
                </Button>
              </div>
              
              {showGraph && (
                <Card className="mt-6">
                  <CardContent className="p-6">
                    <h2 className="text-xl font-semibold mb-4">Backtest Results</h2>
                    <ResponsiveContainer width="100%" height={400}>
                      <LineChart data={results}>
                        <XAxis dataKey="time" />
                        <YAxis
                          tickFormatter={(value) =>
                            value >= 1000000 ? `${value / 1000000}M` : value
                          }
                        />
                        <Tooltip />
                        <CartesianGrid strokeDasharray="3 3" />
                        
                        {/* Dynamically generate lines for each portfolio */}
                        {portfolioKeys.map((portfolioKey, index) => (
                          <Line
                            key={portfolioKey}
                            type="monotone"
                            dataKey={portfolioKey}
                            stroke={`hsl(${(index * 137) % 360}, 70%, 50%)`} // Generate unique color
                            dot={false}
                            name={portfolioKey} // Use portfolio key as legend name
                            strokeWidth={2}
                          />
                        ))}
                        <Legend />
                        <Brush dataKey="time" height={30} stroke="#8884d8" />
                      </LineChart>
                    </ResponsiveContainer>
                  </CardContent>
                </Card>
              )}
            </TabsContent>
          </Tabs>
        </CardContent>
      </Card>
    </div>
  );
}

/*
import React, { useState, useEffect } from "react";
import { Card, CardContent } from "@/components/ui/card";
import { Tabs, TabsList, TabsTrigger, TabsContent } from "@/components/ui/tabs";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Plus, Play } from "lucide-react";
import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid, Brush } from "recharts";

export default function PortfolioConfig() {
 
  const [settings, setSettings] = useState({
    portfolioName: "",
    startDate: "",
    endDate: "",
    initialAmount: "",
    monthlyDeposit: "",
    reinvestmentPolicy: "",
    recurrentInvestmentAmount: "",
    rinvInvestmentNbMonthsFrequency: "",
    rinvInvestmentMonthlyWeekNum: "",
    rinvInvestmentWeekDay: "",
    rinvRebalancingThreshold: "",
    rinvRebalancingFreqMinNbDays: "",
    assets: ""
  });

  const mockData = [
    { strategy: 'mockStrat', time: "Jan", value: 500},
    { strategy: 'mockStrat', time: "Feb", value: 1500},
    { strategy: 'mockStrat', time: "Mar", value: 1000},
    { strategy: 'mockStrat', time: "Apr", value: 1800},
    { strategy: 'mockStrat', time: "May", value: 1400},
    { strategy: 'mockStrat', time: "Jun", value: 2500 },
  ];

  
  const [availableSymbols, setAvailableSymbols] = useState<string[]>([]);
  const [results, setResults] = useState(mockData);
  const [showGraph, setShowGraph] = useState(true);
  const [activeTab, setActiveTab] = useState("settings");


  useEffect(() => {
    // Simulate fetching available symbols from a file
    setAvailableSymbols(["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "CSSPX.MI", "EGLN.L", "HPQ", "TTE.PA"]);
  }, []);

  const [assets, setAssets] = useState(
    Array(5).fill({ symbol: "", allocation: "" })
  );

  const updateSettings = (field: string, value: string) => {
    setSettings({ ...settings, [field]: value });
  };

  const updateAsset = (index: number, field: string, value: string) => {
    setAssets((prevAssets) => {
      const updatedAssets = [...prevAssets]; // Create a new array (immutability)
      updatedAssets[index] = { ...updatedAssets[index], [field]: value }; // Only modify the specific index
      return updatedAssets;
    });
  };

  const addAsset = () => {
    setAssets([...assets, { symbol: "", allocation: "" }]);
  };
  
  const handleRunBacktest = async () => {

    const formattedAssets = assets.reduce((acc, asset) => {
      if (asset.symbol && asset.allocation) {
        acc[asset.symbol] = parseFloat(asset.allocation); // Ensure allocation is a number
      }
      return acc;
    }, {});

    const requestData = {
      portfolioName: [settings.portfolioName],
      startDate: [settings.startDate], // "YYYY-MM-DD"
      endDate: [settings.endDate],   // "YYYY-MM-DD"
      startingAmount: [parseFloat(settings.initialAmount)], // Ensure numeric values
      monthlyDeposit: [parseFloat(settings.monthlyDeposit)],
      reinvestmentPolicy: [settings.reinvestmentPolicy],
      recurrentInvestmentAmount: [parseFloat(settings.recurrentInvestmentAmount)],
      rinvInvestmentNbMonthsFrequency: [parseInt(settings.rinvInvestmentNbMonthsFrequency)],
      rinvInvestmentMonthlyWeekNum: [parseInt(settings.rinvInvestmentMonthlyWeekNum)],
      rinvInvestmentWeekDay: [parseInt(settings.rinvInvestmentWeekDay)],
      rinvRebalancingThreshold: [parseFloat(settings.rinvRebalancingThreshold)],
      rinvRebalancingFreqMinNbDays: [parseInt(settings.rinvRebalancingFreqMinNbDays)],
      rinvAllocations: [[`${JSON.stringify(formattedAssets)}`.replace(/"/g, '').replace('{', '').replace('}', '')]],
      assets: [assets.filter((asset: { symbol: string; allocation: number; }) => asset.symbol && asset.allocation).map((asset: { symbol: string; }) => asset.symbol)]
    };

    console.log("RESQUESTED DATA")
    console.log(JSON.stringify(requestData));
    try {
      const response = await fetch("http://localhost:8080/run-backtest", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(requestData)
      });

      const rawText = await response.text();  // Read raw response as text
      if (!response.ok) {
        throw new Error(`HTTP Error ${response.status}: ${rawText}`);
      }
      console.log("Raw response from backend:", rawText);

      const data = JSON.parse(rawText);  // Attempt to parse JSON
      console.log("Parsed JSON:", data);
      setResults(data); // Replace with actual data from backend
      setShowGraph(true);
    } catch (error) {
      console.error("Error running backtest:", error);
    }
  };

  return (
    <div className="p-6 max-w-4xl mx-auto">
      <Card className="shadow-lg">
        <CardContent className="p-6 space-y-6">
          <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
            <TabsList className="flex gap-2">
              <TabsTrigger value="settings">Settings</TabsTrigger>
              <TabsTrigger value="portfolio-assets">Portfolio Assets</TabsTrigger>
            </TabsList>

            <TabsContent value="settings" className="space-y-4">
            <div className="space-y-4">
              
              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Portfolio Name:</span>
                <Input type="text" value={settings.portfolioName} onChange={(e) => updateSettings("portfolioName", e.target.value)} />
              </div>
              
              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Start Date:</span>
                <Input type="date" value={settings.startDate} onChange={(e) => updateSettings("startDate", e.target.value)} />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">End Date:</span>
                <Input type="date" value={settings.endDate} onChange={(e) => updateSettings("endDate", e.target.value)} />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Initial Amount ($):</span>
                <Input type="number" value={settings.initialAmount} onChange={(e) => updateSettings("initialAmount", e.target.value)} />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Monthly Deposit ($):</span>
                <Input type="number" value={settings.monthlyDeposit} onChange={(e) => updateSettings("monthlyDeposit", e.target.value)} />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Reinvest Dividends?:</span>
                <Input type="text" value={settings.reinvestmentPolicy} onChange={(e) => updateSettings("reinvestmentPolicy", e.target.value)} />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Recurrent Investment Amount ($):</span>
                <Input type="number" value={settings.recurrentInvestmentAmount} onChange={(e) => updateSettings("recurrentInvestmentAmount", e.target.value)} />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Invest every x months ? :</span>
                <Input type="number" value={settings.rinvInvestmentNbMonthsFrequency} onChange={(e) => updateSettings("rinvInvestmentNbMonthsFrequency", e.target.value)} placeholder="Enter the x > 0" />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">On which week number in the month ? :</span>
                <Input type="number" value={settings.rinvInvestmentMonthlyWeekNum} onChange={(e) => updateSettings("rinvInvestmentMonthlyWeekNum", e.target.value)} placeholder="Enter a number between 0 (1st week of month) and 4 (fourth week of the month)" />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">On which week day ? :</span>
                <Input type="number" value={settings.rinvInvestmentWeekDay} onChange={(e) => updateSettings("rinvInvestmentWeekDay", e.target.value)} placeholder="Enter a number between 1 (monday) and 5 (friday)" />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Threshold (%) for rebalancing :</span>
                <Input type="number" value={settings.rinvRebalancingThreshold} onChange={(e) => updateSettings("rinvRebalancingThreshold", e.target.value)} placeholder="Ex: 0.05 : At 5% gap rebalancing is done" />
              </div>

              <div className="flex items-center gap-4">
                <span className="text-gray-600 text-sm w-75">Rebalance only after x days :</span>
                <Input type="number" value={settings.rinvRebalancingFreqMinNbDays} onChange={(e) => updateSettings("rinvRebalancingFreqMinNbDays", e.target.value)} placeholder="Enter x > 0" />
              </div>

              <div className="flex justify-end pt-4">
                <Button className="bg-blue-600 hover:bg-blue-700 text-white" onClick={() => setActiveTab("portfolio-assets")}>
                  Next
                </Button>
              </div>
            </div>
            </TabsContent>

            <TabsContent value="portfolio-assets" className="space-y-4">
            {assets.map((asset, index) => (
              <div key={index} className="flex items-center gap-4">
                <Input
                  list="tickers"
                  value={asset.symbol}
                  onChange={(e) => updateAsset(index, "symbol", e.target.value)}
                  placeholder="Select or type ticker..."
                />
                
                <datalist id="tickers">
                  {availableSymbols.map((symbol) => (
                    <option key={symbol} value={symbol} />
                  ))}
                </datalist>

                <Input 
                  type="number" 
                  value={asset.allocation} 
                  onChange={(e) => updateAsset(index, "allocation", e.target.value)} 
                  placeholder="Asset Allocation (%)" 
                />
              </div>
            ))}
              <Button onClick={addAsset} className="flex items-center gap-2">
                <Plus size={18} /> Add Asset
              </Button>
              <div className="flex justify-center">
                <Button onClick={handleRunBacktest} className="bg-blue-600 hover:bg-blue-700 text-white flex items-center gap-2">
                  <Play size={18} />
                  <span>Run Backtest</span>
                </Button>
              </div>
              {showGraph && (
                <Card>
                  <CardContent className="p-6">
                    <h2 className="text-xl font-semibold">Backtest Results</h2>
                    <ResponsiveContainer width="100%" height={300}>
                      <LineChart data={results}>
                        <XAxis dataKey="time" />
                        <YAxis tickFormatter={(value) => value >= 1000000 ? `${value / 1000000}M` : value}/>
                        <Tooltip />
                        <CartesianGrid strokeDasharray="3 3" />
                        <Line type="monotone" dataKey="value" stroke="#8884d8" dot={false} />
                        <Brush dataKey="time" height={30} stroke="#8884d8" />
                      </LineChart>
                    </ResponsiveContainer>
                  </CardContent>
                </Card>
              )}
            </TabsContent>
          </Tabs>
        </CardContent>
      </Card>
    </div>
  );
}*/

/*
import React, { useState, useEffect } from "react";
import { Card, CardContent } from "@/components/ui/card";
import { Tabs, TabsList, TabsTrigger, TabsContent } from "@/components/ui/tabs";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Plus, Play, PlusCircle } from "lucide-react";
import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid, Brush, Legend } from "recharts";

export default function PortfolioConfig() {
  // Initial empty strategy template
  const emptyStrategy = {
    portfolioName: "",
    startDate: "",
    endDate: "",
    initialAmount: "",
    monthlyDeposit: "",
    reinvestmentPolicy: "",
    recurrentInvestmentAmount: "",
    rinvInvestmentNbMonthsFrequency: "",
    rinvInvestmentMonthlyWeekNum: "",
    rinvInvestmentWeekDay: "",
    rinvRebalancingThreshold: "",
    rinvRebalancingFreqMinNbDays: "",
    assets: Array(5).fill({ symbol: "", allocation: "" })
  };

  // Now we manage an array of strategies
  const [strategies, setStrategies] = useState([{ ...emptyStrategy }]);

  const mockData = [
    { strategy: 'Portfolio 1', time: "Jan", value: 500},
    { strategy: 'Portfolio 1', time: "Feb", value: 1500},
    { strategy: 'Portfolio 1', time: "Mar", value: 1000},
    { strategy: 'Portfolio 1', time: "Apr", value: 1800},
    { strategy: 'Portfolio 1', time: "May", value: 1400},
    { strategy: 'Portfolio 1', time: "Jun", value: 2500 },
  ];

  const [availableSymbols, setAvailableSymbols] = useState([]);
  const [results, setResults] = useState(mockData);
  const [showGraph, setShowGraph] = useState(true);
  const [activeTab, setActiveTab] = useState("settings");

  useEffect(() => {
    // Simulate fetching available symbols from a file
    setAvailableSymbols(["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "CSSPX.MI", "EGLN.L", "HPQ", "TTE.PA"]);
  }, []);

  const addNewStrategy = () => {
    setStrategies([...strategies, { ...emptyStrategy }]);
  };

  const updateSettings = (strategyIndex, field, value) => {
    setStrategies(prevStrategies => {
      const updatedStrategies = [...prevStrategies];
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        [field]: value
      };
      return updatedStrategies;
    });
  };

  const updateAsset = (strategyIndex, assetIndex, field, value) => {
    setStrategies(prevStrategies => {
      const updatedStrategies = [...prevStrategies];
      const updatedAssets = [...updatedStrategies[strategyIndex].assets];
      updatedAssets[assetIndex] = { ...updatedAssets[assetIndex], [field]: value };
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        assets: updatedAssets
      };
      return updatedStrategies;
    });
  };

  const addAsset = (strategyIndex) => {
    setStrategies(prevStrategies => {
      const updatedStrategies = [...prevStrategies];
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        assets: [...updatedStrategies[strategyIndex].assets, { symbol: "", allocation: "" }]
      };
      return updatedStrategies;
    });
  };
  
  const handleRunBacktest = async () => {
    // Create arrays for each field with values from all strategies
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

    console.log("REQUESTED DATA");
    console.log(JSON.stringify(requestData));
    
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
      console.log("Raw response from backend:", rawText);

      const data = JSON.parse(rawText);
      console.log("Parsed JSON:", data);
      setResults(data);
      setShowGraph(true);
    } catch (error) {
      console.error("Error running backtest:", error);
    }
  };

  return (
    <div className="p-6 max-w-6xl mx-auto">
      <Card className="shadow-lg">
        <CardContent className="p-6 space-y-6">
          <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
            <div className="flex justify-between items-center mb-4">
              <TabsList className="flex gap-2">
                <TabsTrigger value="settings">Settings</TabsTrigger>
                <TabsTrigger value="portfolio-assets">Portfolio Assets</TabsTrigger>
              </TabsList>
              <Button 
                onClick={addNewStrategy} 
                className="flex items-center gap-1 bg-blue-600 hover:bg-blue-700 text-white"
              >
                <PlusCircle size={18} />
                <span>Add Strategy</span>
              </Button>
            </div>

            <TabsContent value="settings" className="space-y-4">
              <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                {strategies.map((strategy, strategyIndex) => (
                  <div key={strategyIndex} className="border rounded-lg p-4 space-y-4">
                    <h3 className="font-semibold text-center border-b pb-2">
                      {strategy.portfolioName || `Portfolio ${strategyIndex + 1}`}
                    </h3>
                    
                    <div className="space-y-4">
                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Portfolio Name:</span>
                        <Input 
                          type="text" 
                          value={strategy.portfolioName} 
                          onChange={(e) => updateSettings(strategyIndex, "portfolioName", e.target.value)} 
                        />
                      </div>
                      
                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Start Date:</span>
                        <Input 
                          type="date" 
                          value={strategy.startDate} 
                          onChange={(e) => updateSettings(strategyIndex, "startDate", e.target.value)} 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">End Date:</span>
                        <Input 
                          type="date" 
                          value={strategy.endDate} 
                          onChange={(e) => updateSettings(strategyIndex, "endDate", e.target.value)} 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Initial Amount ($):</span>
                        <Input 
                          type="number" 
                          value={strategy.initialAmount} 
                          onChange={(e) => updateSettings(strategyIndex, "initialAmount", e.target.value)} 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Monthly Deposit ($):</span>
                        <Input 
                          type="number" 
                          value={strategy.monthlyDeposit} 
                          onChange={(e) => updateSettings(strategyIndex, "monthlyDeposit", e.target.value)} 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Reinvest Dividends?:</span>
                        <Input 
                          type="text" 
                          value={strategy.reinvestmentPolicy} 
                          onChange={(e) => updateSettings(strategyIndex, "reinvestmentPolicy", e.target.value)} 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Recurrent Investment Amount ($):</span>
                        <Input 
                          type="number" 
                          value={strategy.recurrentInvestmentAmount} 
                          onChange={(e) => updateSettings(strategyIndex, "recurrentInvestmentAmount", e.target.value)} 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Invest every x months?:</span>
                        <Input 
                          type="number" 
                          value={strategy.rinvInvestmentNbMonthsFrequency} 
                          onChange={(e) => updateSettings(strategyIndex, "rinvInvestmentNbMonthsFrequency", e.target.value)} 
                          placeholder="Enter x > 0" 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Week number in month:</span>
                        <Input 
                          type="number" 
                          value={strategy.rinvInvestmentMonthlyWeekNum} 
                          onChange={(e) => updateSettings(strategyIndex, "rinvInvestmentMonthlyWeekNum", e.target.value)} 
                          placeholder="0-4" 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Week day:</span>
                        <Input 
                          type="number" 
                          value={strategy.rinvInvestmentWeekDay} 
                          onChange={(e) => updateSettings(strategyIndex, "rinvInvestmentWeekDay", e.target.value)} 
                          placeholder="1-5" 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Rebalancing threshold (%):</span>
                        <Input 
                          type="number" 
                          value={strategy.rinvRebalancingThreshold} 
                          onChange={(e) => updateSettings(strategyIndex, "rinvRebalancingThreshold", e.target.value)} 
                          placeholder="Ex: 0.05" 
                        />
                      </div>

                      <div className="flex flex-col">
                        <span className="text-gray-600 text-sm mb-1">Rebalance after x days:</span>
                        <Input 
                          type="number" 
                          value={strategy.rinvRebalancingFreqMinNbDays} 
                          onChange={(e) => updateSettings(strategyIndex, "rinvRebalancingFreqMinNbDays", e.target.value)} 
                          placeholder="Enter x > 0" 
                        />
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            </TabsContent>

            <TabsContent value="portfolio-assets" className="space-y-4">
              <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                {strategies.map((strategy, strategyIndex) => (
                  <div key={strategyIndex} className="border rounded-lg p-4 space-y-4">
                    <h3 className="font-semibold text-center border-b pb-2">
                      {strategy.portfolioName || `Portfolio ${strategyIndex + 1}`}
                    </h3>
                    
                    <div className="space-y-4">
                      {strategy.assets.map((asset, assetIndex) => (
                        <div key={assetIndex} className="space-y-2">
                          <Input
                            list={`tickers-${strategyIndex}`}
                            value={asset.symbol}
                            onChange={(e) => updateAsset(strategyIndex, assetIndex, "symbol", e.target.value)}
                            placeholder="Select or type ticker..."
                          />
                          
                          <datalist id={`tickers-${strategyIndex}`}>
                            {availableSymbols.map((symbol) => (
                              <option key={symbol} value={symbol} />
                            ))}
                          </datalist>

                          <Input 
                            type="number" 
                            value={asset.allocation} 
                            onChange={(e) => updateAsset(strategyIndex, assetIndex, "allocation", e.target.value)} 
                            placeholder="Asset Allocation (%)" 
                          />
                        </div>
                      ))}
                      <Button 
                        onClick={() => addAsset(strategyIndex)} 
                        className="w-full flex items-center justify-center gap-2"
                      >
                        <Plus size={18} /> Add Asset
                      </Button>
                    </div>
                  </div>
                ))}
              </div>
              
              <div className="flex justify-center mt-6">
                <Button 
                  onClick={handleRunBacktest} 
                  className="bg-blue-600 hover:bg-blue-700 text-white flex items-center gap-2 px-6 py-2"
                >
                  <Play size={18} />
                  <span>Run Backtest</span>
                </Button>
              </div>
              
              {showGraph && (
                <Card className="mt-6">
                  <CardContent className="p-6">
                    <h2 className="text-xl font-semibold mb-4">Backtest Results</h2>
                    <ResponsiveContainer width="100%" height={400}>
                      <LineChart data={results}>
                        <XAxis dataKey="time" />
                        <YAxis tickFormatter={(value) => value >= 1000000 ? `${value / 1000000}M` : value}/>
                        <Tooltip />
                        <CartesianGrid strokeDasharray="3 3" />
                        {strategies.map((strategy, index) => (
                          <Line 
                            key={index}
                            type="monotone" 
                            dataKey="value" 
                            stroke={`hsl(${(index * 137) % 360}, 70%, 50%)`} 
                            dot={false} 
                            name={strategy.portfolioName || `Portfolio ${index + 1}`}
                            strokeWidth={2}
                          />
                        ))}
                        <Legend />
                        <Brush dataKey="time" height={30} stroke="#8884d8" />
                      </LineChart>
                    </ResponsiveContainer>
                  </CardContent>
                </Card>
              )}
            </TabsContent>
          </Tabs>
        </CardContent>
      </Card>
    </div>
  );
}*/