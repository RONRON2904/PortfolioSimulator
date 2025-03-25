import React from "react";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Plus } from "lucide-react";
import { Trash } from "lucide-react";

export default function PortfolioAssets({ strategies, setStrategies, availableSymbols }) {
  const updateAsset = (strategyIndex: number, assetIndex: number, field: string, value: string) => {
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

  const addAsset = (strategyIndex: number) => {
    setStrategies(prevStrategies => {
      const updatedStrategies = [...prevStrategies];
      updatedStrategies[strategyIndex] = {
        ...updatedStrategies[strategyIndex],
        assets: [...updatedStrategies[strategyIndex].assets, { symbol: "", allocation: "" }]
      };
      return updatedStrategies;
    });
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
          <h3 className="font-semibold text-center mb-4">Portfolio {index + 1}</h3>
          <div className="space-y-4">
            {strategy.assets.map((asset, assetIndex) => (
              <div key={assetIndex} className="flex items-center gap-4">
                <Input
                  list={`tickers-${index}`}
                  value={asset.symbol}
                  onChange={(e) => {
                    updateAsset(index, assetIndex, "symbol", e.target.value)}}
                  placeholder="Select or type ticker..."
                  className="flex-1"
                />
                <datalist id={`tickers-${index}`}>
                  {availableSymbols.map((symbol) => (
                    <option key={symbol} value={symbol} />
                  ))}
                </datalist>
                <Input
                  type="number"
                  value={asset.allocation}
                  onChange={(e) =>
                    updateAsset(index, assetIndex, "allocation", e.target.value)
                  }
                  placeholder="Asset Allocation (%)"
                  className="flex-1"
                />
              </div>
            ))}
            <div className="flex justify-end">
              <button
                onClick={() => addAsset(index)}
                className="bg-black hover:bg-gray-800 text-white px-2 py-1 rounded-md shadow-md flex items-center gap-1"
              >
                <Plus size={16} />
                Add Row
              </button>
            </div>
          </div>
        </div>
      ))}
    </div>
  );
}
