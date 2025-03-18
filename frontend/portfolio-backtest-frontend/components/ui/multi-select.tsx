import React from "react";
import Select, { MultiValue } from "react-select";

type OptionType = { label: string; value: string };

type MultiSelectProps = {
  options: OptionType[];
  value: string[];
  onChange: (selectedOptions: string[]) => void;
  placeholder?: string;
};

export function MultiSelect({ options, value, onChange, placeholder = "Select options" }: MultiSelectProps) {
  const handleChange = (selectedOptions: MultiValue<OptionType>) => {
    onChange(selectedOptions.map((opt) => opt.value));
  };

  return (
    <Select
      isMulti
      options={options}
      value={options.filter((opt) => value.includes(opt.value))}
      onChange={handleChange}
      placeholder={placeholder}
      className="basic-multi-select"
      classNamePrefix="select"
    />
  );
}
