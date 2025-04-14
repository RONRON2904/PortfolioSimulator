import React, { useState } from "react";
import { Input } from "@/components/ui/input";
import Checkbox from "@/components/ui/checkbox";
import { Trash, Copy, ChevronDown, ChevronUp } from "lucide-react";

export default function StrategySettings({ strategies, setStrategies }) {
  const [showRecurring, setShowRecurring] = useState(false);
  const [showWithdraw, setShowWithdraw] = useState(false);
  const [showTechinds, setShowTechinds] = useState(false);

  const updateSettings = (strategyIndex, field, value) => {
    setStrategies((prevStrategies) => {
      const updatedStrategies = [...prevStrategies];
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        [field]: value,
      };
      return updatedStrategies;
    });
  };

  const replicateStrategy = (index) => {
    const strategyToReplicate = {
      ...strategies[index],
      assets: strategies[index].assets.map((asset) => ({ ...asset })),
    };
    setStrategies([...strategies, strategyToReplicate]);
  };

  return (
    <div className="flex gap-4 overflow-x-auto">
      {strategies.map((strategy, index) => (
        <div
          key={index}
          className="relative transition-all duration-300 ease-in-out border rounded-lg p-4 bg-white shadow-md"
          style={{
            width: `calc(100% / ${Math.min(strategies.length, 2)})`,
            minWidth: "600px",
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
          <button
            onClick={() => replicateStrategy(index)}
            className="text-white bg-blue-500 hover:bg-blue-400 rounded-full p-2"
          >
            <Copy size={16} />
          </button>
          <h3 className="font-semibold text-center mb-4">Portfolio {index + 1}</h3>
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
            <label className="w-1/3">Initial Amount ($)</label>
            <Input
              type="number"
              value={strategy.initialAmount}
              onChange={(e) => updateSettings(index, "initialAmount", e.target.value)}
              className="flex-1 border rounded p-2"
            />
          </div>
            {/* Toggle button for withdrawal investments */}
            <button
              onClick={() => setShowWithdraw(!showWithdraw)}
              className="w-full text-left text-blue-500 font-semibold flex items-center gap-2 mt-4"
            >
              {showWithdraw ? <ChevronUp size={16} /> : <ChevronDown size={16} />} Add Withdrawal Parameters
            </button>
            
            {showWithdraw && (
              <div className="space-y-4 border p-4 rounded-lg bg-gray-100">
                <div className="flex items-center gap-4">
                  <label className="w-1/3">Withdrawal Annual %</label>
                  <Input
                    type="number"
                    value={strategy.rinvWithdrawalPct}
                    onChange={(e) => updateSettings(index, "rinvWithdrawalPct", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="Enter x >= 0" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">Withdrawal Annual Amount</label>
                  <Input
                    type="number"
                    value={strategy.rinvWithdrawalAmount}
                    onChange={(e) => updateSettings(index, "rinvWithdrawalAmount", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="Enter x >= 0" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">Withdraw every x Month</label>
                  <Input
                    type="number"
                    value={strategy.rinvWithdrawalNbMonthsFrequency}
                    onChange={(e) => updateSettings(index, "rinvWithdrawalNbMonthsFrequency", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="Enter x >= 0" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">Withdraw on which week number of the month?</label>
                  <Input
                    type="number"
                    value={strategy.rinvWithdrawalMonthlyWeekNum}
                    onChange={(e) => updateSettings(index, "rinvWithdrawalMonthlyWeekNum", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="0-4" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">Withdraw on which week day?</label>
                  <Input
                    type="number"
                    value={strategy.rinvWithdrawalWeekDay}
                    onChange={(e) => updateSettings(index, "rinvWithdrawalWeekDay", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="1-5" 
                  />
                </div>
              </div>
            )}

            {/* Toggle button for recurring investments */}
            <button
              onClick={() => setShowRecurring(!showRecurring)}
              className="w-full text-left text-blue-500 font-semibold flex items-center gap-2 mt-4"
            >
              {showRecurring ? <ChevronUp size={16} /> : <ChevronDown size={16} />} Add Recurring Parameters
            </button>
            
            {showRecurring && (
              <div className="space-y-4 border p-4 rounded-lg bg-gray-100">
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
                    value={strategy.rinvInvestmentAmount}
                    onChange={(e) => updateSettings(index, "rinvInvestmentAmount", e.target.value)}
                    className="flex-1 border rounded p-2"
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">Invest every x months:</label>
                  <Input
                    type="number"
                    value={strategy.rinvInvestmentNbMonthsFrequency}
                    onChange={(e) => updateSettings(index, "rinvInvestmentNbMonthsFrequency", e.target.value)}
                    className="flex-1 border rounded p-2"
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">On which week number of the month?</label>
                  <Input
                    type="number"
                    value={strategy.rinvInvestmentMonthlyWeekNum}
                    onChange={(e) => updateSettings(index, "rinvInvestmentMonthlyWeekNum", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="0-4" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">On which week day?</label>
                  <Input
                    type="number"
                    value={strategy.rinvInvestmentWeekDay}
                    onChange={(e) => updateSettings(index, "rinvInvestmentWeekDay", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="1-5" 
                  />
                </div>
              </div>
            )}
            {/* Toggle button for technical indicators investments */}
            <button
              onClick={() => setShowTechinds(!showTechinds)}
              className="w-full text-left text-blue-500 font-semibold flex items-center gap-2 mt-4"
            >
              {showTechinds ? <ChevronUp size={16} /> : <ChevronDown size={16} />} Add Technical Indicators
            </button>
            
            {showTechinds && (
              <div className="space-y-4 border p-4 rounded-lg bg-gray-100">
                <div className="flex items-center gap-4">
                  <label className="w-1/3">SMA window period</label>
                  <Input
                    type="number"
                    value={strategy.techindSmaWindow}
                    onChange={(e) => updateSettings(index, "techindSmaWindow", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="Enter x > 0" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">RSI window period</label>
                  <Input
                    type="number"
                    value={strategy.techindRsiWindow}
                    onChange={(e) => updateSettings(index, "techindRsiWindow", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="Enter x > 0" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">RSI buy threshold</label>
                  <Input
                    type="number"
                    value={strategy.techindRsiBuyThreshold}
                    onChange={(e) => updateSettings(index, "techindRsiBuyThreshold", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="0-4" 
                  />
                </div>
                <div className="flex items-center gap-4">
                  <label className="w-1/3">RSI sell threshold</label>
                  <Input
                    type="number"
                    value={strategy.techindRsiSellThreshold}
                    onChange={(e) => updateSettings(index, "techindRsiSellThreshold", e.target.value)}
                    className="flex-1 border rounded p-2"
                    placeholder="1-5" 
                  />
                </div>
              </div>
            )}
          
            <div className="flex items-center justify-center gap-4">
              <div className="flex items-center gap-4">
                <label className="text-sm">Reinvest Dividends?</label>
                <Checkbox
                  checked={strategy.reinvestmentPolicy}
                  onChange={(checked) => updateSettings(index, "reinvestmentPolicy", checked)}
                />
              </div>
              
              <div className="flex items-center gap-4">
                <label className="text-sm">Flat Tax</label>
                <Input
                  type="number"
                  value={strategy.tax}
                  onChange={(e) => updateSettings(index, "tax", e.target.value)}
                  className="w-20 border rounded p-1"
                  placeholder="0.3"
                />
              </div>
              
              <div className="flex items-center gap-4">
                <label className="text-sm">Fees per trade</label>
                <Input
                  type="number"
                  value={strategy.tradeFees}
                  onChange={(e) => updateSettings(index, "tradeFees", e.target.value)}
                  className="w-20 border rounded p-1"
                  placeholder="≥ 0"
                />
              </div>
            </div>
            
          </div>
        </div>
      ))}
    </div>
  );
}
