"use client";

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
  });

  const mockData = [
    { time: "Jan", value: 2000 },
    { time: "Feb", value: 1100 },
    { time: "Mar", value: 1200 },
    { time: "Apr", value: 1800 },
  ];

  
  const [availableSymbols, setAvailableSymbols] = useState<string[]>([]);
  const [results, setResults] = useState(mockData);
  const [showGraph, setShowGraph] = useState(false);
  const [activeTab, setActiveTab] = useState("settings");


  useEffect(() => {
    // Simulate fetching available symbols from a file
    setAvailableSymbols(["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "CSSPX.MI", "EGLN.L", "HPQ", "TTE.PA"]);
  }, []);

  const [assets, setAssets] = useState(
    Array(5).fill({ symbol: "", allocation: "" })
  );

  const updateSettings = (field: string, value: any) => {
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
      portfolioName: settings.portfolioName,
      startDate: settings.startDate, // "YYYY-MM-DD"
      endDate: settings.endDate,   // "YYYY-MM-DD"
      startingAmount: parseFloat(settings.initialAmount), // Ensure numeric values
      monthlyDeposit: parseFloat(settings.monthlyDeposit),
      reinvestmentPolicy: settings.reinvestmentPolicy,
      recurrentInvestmentAmount: parseFloat(settings.recurrentInvestmentAmount),
      rinvInvestmentNbMonthsFrequency: parseInt(settings.rinvInvestmentNbMonthsFrequency),
      rinvInvestmentMonthlyWeekNum: parseInt(settings.rinvInvestmentMonthlyWeekNum),
      rinvInvestmentWeekDay: parseInt(settings.rinvInvestmentWeekDay),
      rinvRebalancingThreshold: parseFloat(settings.rinvRebalancingThreshold),
      rinvRebalancingFreqMinNbDays: parseInt(settings.rinvRebalancingFreqMinNbDays),
      rinvAllocations: formattedAssets
    };

    console.log(requestData);
    try {
      const response = await fetch("http://localhost:8080/run-backtest", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(requestData)
      });

      const rawText = await response.text();  // Read raw response as text
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

            {/* Settings Tab */}
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

            {/* Portfolio Assets Tab */}
            <TabsContent value="portfolio-assets" className="space-y-4">
            {assets.map((asset, index) => (
              <div key={index} className="flex items-center gap-4">
                {/* Input with datalist for search and free typing */}
                <Input
                  list="tickers"
                  value={asset.symbol}
                  onChange={(e) => updateAsset(index, "symbol", e.target.value)}
                  placeholder="Select or type ticker..."
                />
                
                {/* Datalist containing available tickers */}
                <datalist id="tickers">
                  {availableSymbols.map((symbol) => (
                    <option key={symbol} value={symbol} />
                  ))}
                </datalist>

                {/* Allocation input remains unchanged */}
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
}