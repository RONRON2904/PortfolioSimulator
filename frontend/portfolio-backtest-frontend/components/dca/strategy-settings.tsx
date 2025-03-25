import React from "react";
import { Input } from "@/components/ui/input";
import { Trash } from "lucide-react";

export default function StrategySettings({ strategies, setStrategies }) {
  const updateSettings = (index, field, value) => {
    const updatedStrategies = [...strategies];
    updatedStrategies[index][field] = value;
    setStrategies(updatedStrategies);
  };

  return (
    <div className="flex gap-4 overflow-x-auto">
    {strategies.map((strategy, index) => (
      <div
        key={index}
        className={`relative transition-all duration-300 ease-in-out border rounded-lg p-4 bg-white shadow-md`}
        style={{
          width: `calc(100% / ${Math.min(strategies.length, 2)})`, // Limit to max 2 visible cards
          minWidth: "400px", // Set a minimum width for cards
        }}
      >
        <button
          onClick={() =>
            setStrategies((prev) => prev.filter((_, i) => i !== index))
          }
          className="absolute top-2 right-2 text-gray-500 hover:text-red-600 transition-colors"
        >
          <Trash size={20} />
        </button>
        <h3 className="font-semibold text-center mb-4">
          Portfolio {index + 1}
        </h3>
        <div className="space-y-4">
          <div className="flex items-center gap-4">
            <label className="w-1/3">Portfolio Name</label>
            <Input
              type="text"
              value={strategy.portfolioName}
              onChange={(e) => updateSettings(index, "portfolioName", e.target.value)}
              className="flex-1 border rounded p-2"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Start Date</label>
            <Input
              type="date"
              value={strategy.startDate}
              onChange={(e) => updateSettings(index, "startDate", e.target.value)}
              className="flex-1 border rounded p-2"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">End Date</label>
            <Input
              type="date"
              value={strategy.endDate}
              onChange={(e) => updateSettings(index, "endDate", e.target.value)}
              className="flex-1 border rounded p-2"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Initial Amount</label>
            <Input
              type="number"
              value={strategy.initialAmount}
              onChange={(e) => updateSettings(index, "initialAmount", e.target.value)}
              className="flex-1 border rounded p-2"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Monthly Deposit ($):</label>
            <Input
              type="number"
              value={strategy.monthlyDeposit}
              onChange={(e) => updateSettings(index, "monthlyDeposit", e.target.value)}
              className="flex-1 border rounded p-2"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Recurrent Investment Amount ($):</label>
            <Input
              type="number"
              value={strategy.recurrentInvestmentAmount}
              onChange={(e) => updateSettings(index, "recurrentInvestmentAmount", e.target.value)}
              className="flex-1 border rounded p-2"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Reinvest Dividends?:</label>
            <Input
              type="text"
              value={strategy.reinvestmentPolicy}
              onChange={(e) => updateSettings(index, "reinvestmentPolicy", e.target.value)}
              className="flex-1 border rounded p-2"
              placeholder="true / false"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Invest every x months?:</label>
            <Input
              type="number"
              value={strategy.rinvInvestmentNbMonthsFrequency}
              onChange={(e) => updateSettings(index, "rinvInvestmentNbMonthsFrequency", e.target.value)}
              className="flex-1 border rounded p-2"
              placeholder="Enter x > 0"
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Week number in month:</label>
            <Input
              type="number"
              value={strategy.rinvInvestmentMonthlyWeekNum}
              onChange={(e) => updateSettings(index, "rinvInvestmentMonthlyWeekNum", e.target.value)}
              className="flex-1 border rounded p-2"
              placeholder="0-4" 
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Week day:</label>
            <Input
              type="number"
              value={strategy.rinvInvestmentWeekDay}
              onChange={(e) => updateSettings(index, "rinvInvestmentWeekDay", e.target.value)}
              className="flex-1 border rounded p-2"
              placeholder="1-5" 
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Rebalancing threshold (%)</label>
            <Input
              type="number"
              value={strategy.rinvRebalancingThreshold}
              onChange={(e) => updateSettings(index, "rinvRebalancingThreshold", e.target.value)}
              className="flex-1 border rounded p-2"
              placeholder="Ex: 0.05" 
            />
          </div>
          <div className="flex items-center gap-4">
            <label className="w-1/3">Rebalance after x days:</label>
            <Input
              type="number"
              value={strategy.rinvRebalancingFreqMinNbDays}
              onChange={(e) => updateSettings(index, "rinvRebalancingFreqMinNbDays", e.target.value)}
              className="flex-1 border rounded p-2"
              placeholder="Enter x > 0" 
            />
          </div>
        </div>
      </div>
    ))}
  </div>
  );
}
