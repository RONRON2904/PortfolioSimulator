import React from "react";
import DatePicker from "react-datepicker";
import "react-datepicker/dist/react-datepicker.css";

type DatePickerProps = {
  selected: Date | null;
  onChange: (date: Date | null) => void;
  placeholderText?: string;
};

export function CustomDatePicker({ selected, onChange, placeholderText }: DatePickerProps) {
  return (
    <DatePicker
      selected={selected}
      onChange={onChange}
      className="border rounded-md p-2 w-full"
      placeholderText={placeholderText}
      dateFormat="yyyy-MM-dd"
    />
  );
}