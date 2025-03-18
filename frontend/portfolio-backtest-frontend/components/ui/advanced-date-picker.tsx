import React from "react";
import dayjs, { Dayjs } from "dayjs";
import { LocalizationProvider } from "@mui/x-date-pickers/LocalizationProvider";
import { AdapterDayjs } from "@mui/x-date-pickers/AdapterDayjs";
import { DatePicker } from "@mui/x-date-pickers/DatePicker";

type AdvancedDatePickerProps = {
  selected: Dayjs | null;
  onChange: (date: Dayjs | null) => void;
  label?: string;
};

export function AdvancedDatePicker({ selected, onChange, label }: AdvancedDatePickerProps) {
  return (
    <LocalizationProvider dateAdapter={AdapterDayjs}>
      <DatePicker
        label={label}
        value={selected}
        onChange={onChange}
        views={["year", "month", "day"]} // ✅ Allows selection of year -> month -> day
        slotProps={{
          textField: { fullWidth: true, variant: "outlined" },
        }}
      />
    </LocalizationProvider>
  );
}
