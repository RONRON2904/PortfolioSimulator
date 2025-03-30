"use client";

import React, { useState, useEffect } from "react";
import { Plus, Play, PlusCircle } from "lucide-react";
import { Card, CardContent } from "@/components/ui/card";
import { Tabs, TabsList, TabsTrigger, TabsContent } from "@/components/ui/tabs";
import { Button } from "@/components/ui/button";
import StrategySettings from "@/components/dca/strategy-settings";
import PortfolioAssets from "@/components/dca/portfolio-assets";
import BacktestResultsChart from "@/components/dca/backtest-graph";
import { handleRunBacktest } from "@/components/dca/run-backtest";

export default function PortfolioConfig() {
  const emptyStrategy = { nbStrategy: "",
                          tax: "0.0",
                          tradeFees: "0.0",
                          portfolioName: "",
                          startDate: "",
                          endDate: "",
                          initialAmount: "",
                          monthlyDeposit: "0",
                          reinvestmentPolicy: "true",
                          rinvInvestmentAmount: "0",
                          rinvInvestmentNbMonthsFrequency: "1",
                          rinvInvestmentMonthlyWeekNum: "0",
                          rinvInvestmentWeekDay: "1",
                          rinvRebalancingThreshold: "0",
                          rinvRebalancingFreqMinNbDays: "0",
                          rinvWithdrawalPct: "0.0",
                          rinvWithdrawalNbMonthsFrequency: "0",
                          rinvWithdrawalMonthlyWeekNum: "0",
                          rinvWithdrawalWeekDay: "1",
                          assets: Array(3).fill({ symbol: "", allocation: "" }) 
                        };
    const [strategies, setStrategies] = useState([{ ...emptyStrategy }]);
    const [availableSymbols, setAvailableSymbols] = useState<string[]>([]);
    const [results, setResults] = useState([
      { time: "2025-01-01", portfolio1: 500, portfolio2: 500 },
      { time: "2025-02-01", portfolio1: 1500, portfolio2: 300 },
      { time: "2025-03-01", portfolio1: 1000, portfolio2: 1000 },
      { time: "2025-04-01", portfolio1: 1800, portfolio2: 2500 },
      { time: "2025-05-01", portfolio1: 1400, portfolio2: 1000 },
      { time: "2025-06-01", portfolio1: 2500, portfolio2: 3500 },
    ]);
    const [showGraph, setShowGraph] = useState(false);
    const [activeTab, setActiveTab] = useState("settings");
  
    useEffect(() => {
      setAvailableSymbols(["AAPL", "GOOGL", "MSFT","TSLA","AMZN","CSSPX.MI","IDUS.L","EGLN.L","HPQ","TTE.PA","WMT","NVDA"]);
    }, []);
  
    const addNewStrategy = () => {
      setStrategies([...strategies, { ...emptyStrategy }]);
    };

    return (
      <div className="p-4 max-w-7xl mx-auto">
        <Card className="shadow-lg">
          <CardContent className="p-6 space-y-2">
            <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
              <div className="flex justify-between items-center mb-4">
                <TabsList className="flex gap-2">
                  <TabsTrigger value="settings">Settings</TabsTrigger>
                  <TabsTrigger value="portfolio-assets">Portfolio Assets</TabsTrigger>
                </TabsList>
                <div className="flex justify-center mt-4">
                  <Button
                    onClick={addNewStrategy}
                    className="flex items-center gap-1 bg-blue-500 hover:bg-blue-400 text-white px-1 py-1"
                  >
                    <PlusCircle size={18} />
                    <span>Add Strategy</span>
                  </Button>
                </div>
              </div>
  
              <TabsContent value="settings">
                <StrategySettings strategies={strategies} setStrategies={setStrategies}/>
              </TabsContent>
  
              <TabsContent value="portfolio-assets">
                <PortfolioAssets strategies={strategies} setStrategies={setStrategies} availableSymbols={availableSymbols}/>
                <div className="flex justify-center mt-6">
                  <Button
                    onClick={() => handleRunBacktest(strategies, setResults, setShowGraph)}
                    className="bg-blue-600 hover:bg-blue-700 text-white flex items-center gap-2 px-6 py-2"
                  >
                    <Play size={18} />
                    <span>Run Backtest</span>
                  </Button>
                </div>
                {showGraph && <BacktestResultsChart results={results} />}
              </TabsContent>
            </Tabs>
          </CardContent>
        </Card>
      </div>
    );
  }