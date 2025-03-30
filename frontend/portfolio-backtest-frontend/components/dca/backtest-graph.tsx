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

export default function BacktestResultsChart({ results }) {
  const portfolioKeys = Object.keys(results[0]).filter((key) => key !== "time");

  const allPLValues = results.flatMap((data) =>
    portfolioKeys.map((key) => data[key][1])
  );
  const minPL = Math.min(...allPLValues);
  const maxPL = Math.max(...allPLValues);

  return (
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

        <h2 className="text-xl font-semibold mt-6 mb-4">Profit &amp; Loss</h2>
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
      </CardContent>
    </Card>
  );
}