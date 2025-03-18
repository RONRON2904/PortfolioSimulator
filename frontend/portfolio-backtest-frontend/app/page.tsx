"use client"

import React from "react";
import { Card, CardContent } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { LineChart, TrendingUp, Settings } from "lucide-react";
import { useRouter } from "next/navigation";

const BacktestingHome = () => {
  const router = useRouter();

  const backtestOptions = [
    {
      title: "DCA Backtesting",
      description: "Test a dollar-cost averaging strategy with various parameters.",
      icon: <LineChart size={40} className="text-blue-500" />, 
      route: "dca_view"
    },
    {
      title: "Trend-Following Backtesting",
      description: "Evaluate trend-following strategies like moving averages and RSI.",
      icon: <TrendingUp size={40} className="text-green-500" />, 
      route: "/backtest/trend-following"
    },
    {
      title: "Custom Strategy",
      description: "Set up your own backtesting strategy with advanced parameters.",
      icon: <Settings size={40} className="text-gray-500" />, 
      route: "/backtest/custom"
    }
  ];

  return (
    <div className="p-6 space-y-6">
      <h1 className="text-3xl font-bold text-center">Portfolio Backtesting</h1>
      <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
        {backtestOptions.map((option, index) => (
          <Card key={index} className="shadow-lg p-6 flex flex-col items-center text-center">
            {option.icon}
            <h2 className="text-xl font-semibold mt-4">{option.title}</h2>
            <p className="text-sm text-gray-600 mt-2">{option.description}</p>
            <Button className="mt-4" onClick={() => router.push(option.route)}>
              Explore
            </Button>
          </Card>
        ))}
      </div>
    </div>
  );
};

export default BacktestingHome;