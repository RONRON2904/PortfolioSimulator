import React from "react";
import { Card, CardContent } from "@/components/ui/card";
import {
  ResponsiveContainer,
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  CartesianGrid,
  Legend,
  Brush,
  ReferenceArea,
} from "recharts";

const BacktestResultsChart = React.memo(function BacktestResultsChart({ results }) {
  const portfolioKeys = Object.keys(results[0]["strat_values"][0]).filter((key) => key !== "time");

  const allPLValues = results[0]["strat_values"].flatMap((data) =>
    portfolioKeys.map((key) => data[key][1])
  );
  const minPL = Math.min(...allPLValues);
  const maxPL = Math.max(...allPLValues);

  const lastAthPctChangeValues = results[0]["strat_values"].flatMap((data) => portfolioKeys.map((key) => data[key][2]));
  const minAthPctChange = Math.min(...lastAthPctChangeValues);
  const maxAthPctChange = Math.max(...lastAthPctChangeValues);


  return (
    <Card className="mt-6">
      <CardContent className="p-6">
        <h2 className="text-xl font-semibold mb-4">Backtest Results</h2>
        <ResponsiveContainer width="100%" height={400}>
          <LineChart data={results[0]["strat_values"]}>
            <XAxis dataKey="time" />
            <YAxis
              tickFormatter={(value) =>
                value >= 1000000 ? `${value / 1000000}M` : value
              }
            />
            <Tooltip />
            <CartesianGrid strokeDasharray="3 3" />
            {portfolioKeys.map((key, index) => (
              <Line
                key={key}
                type="monotone"
                dataKey={`${key}[0]`} // Adjusted to plot value part of [value, pl]
                stroke={`hsl(${(index * 137) % 360}, 70%, 50%)`}
                dot={false}
                name={key} // Use portfolio key as legend name
                strokeWidth={2}
              />
            ))}
            <Legend />
            <Brush dataKey="time" height={30} stroke="#8884d8" />
          </LineChart>
        </ResponsiveContainer>
        {/* Strategy Performance Summary */}
        <div className="mt-4 space-y-2">
          {results[1]?.strat_perfs?.map((perf) => (
            <div key={perf.strategy_name} className="text-sm text-gray-700">
              <strong>{perf.strategy_name}</strong>: 
              &nbsp;Total Return: {(perf.total_returns * 100).toFixed(2)}%,
              &nbsp;XIRR: {(perf.xirr * 100).toFixed(2)}%,
              &nbsp;Total Investments: {perf.total_investments} $
            </div>
          ))}
        </div>
        <h2 className="text-xl font-semibold mt-6 mb-4">Profit &amp; Loss</h2>
        <ResponsiveContainer width="100%" height={400}>
          <LineChart data={results[0]["strat_values"]}>
            <XAxis dataKey="time" />
            <YAxis
              tickFormatter={(value) =>
                value >= 1000000 ? `${value / 1000000}M` : value
              }
            />
            <Tooltip />
            <CartesianGrid strokeDasharray="3 3" />
            {/* Add green background for positive values */}
            <ReferenceArea
              y1={0}
              y2={maxPL}
              fill="lightgreen"
              fillOpacity={0.3}
              ifOverflow="extendDomain"
            />
            {/* Add red background for negative values */}
            <ReferenceArea
              y1={minPL}
              y2={0}
              fill="lightcoral"
              fillOpacity={0.3}
              ifOverflow="extendDomain"
            />
            {portfolioKeys.map((key, index) => (
              <Line
                key={key}
                type="monotone"
                dataKey={(data) => (data[key] ? data[key][1] : null)} // Access the P&L value
                stroke={`hsl(${(index * 137) % 360}, 70%, 50%)`}
                dot={false}
                name={`${key} (P&L)`} // Use portfolio key as legend name
                strokeWidth={2}
              />
            ))}
            <Legend />
            <Brush dataKey="time" height={30} stroke="#8884d8" />
          </LineChart>
        </ResponsiveContainer>

        <h2 className="text-xl font-semibold mt-6 mb-4">Last ATH % Change</h2>
        <ResponsiveContainer width="100%" height={400}>
          <LineChart data={results[0]["strat_values"]}>
            <XAxis dataKey="time" />
            <YAxis
              tickFormatter={(value) =>
                value >= 1000000 ? `${value / 1000000}M` : value
              }
            />
            <Tooltip />
            <CartesianGrid strokeDasharray="3 3" />
            {portfolioKeys.map((key, index) => (
              <Line
                key={key}
                type="monotone"
                dataKey={(data) => (data[key] ? 100* data[key][2] : null)} // Access the P&L value
                stroke={`hsl(${(index * 137) % 360}, 70%, 50%)`}
                dot={false}
                name={`${key} (% change since last ATH)`} // Use portfolio key as legend name
                strokeWidth={2}
              />
            ))}
            <Legend />
            <Brush dataKey="time" height={30} stroke="#8884d8" />
          </LineChart>
        </ResponsiveContainer>
      </CardContent>
    </Card>
  );
});
export default BacktestResultsChart;