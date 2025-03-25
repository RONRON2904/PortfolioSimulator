import React from "react";
import { Card, CardContent } from "@/components/ui/card";
import { ResponsiveContainer, LineChart, Line, XAxis, YAxis, Tooltip, CartesianGrid, Legend, Brush } from "recharts";

export default function BacktestResultsChart({ results }) {
  const portfolioKeys = Object.keys(results[0]).filter((key) => key !== "time");

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
                dataKey={key}
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
      </CardContent>
    </Card>
  );
}
