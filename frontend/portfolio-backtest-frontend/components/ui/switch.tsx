import * as React from "react";
import * as SwitchPrimitive from "@radix-ui/react-switch";
import { cn } from "@/lib/utils";

export interface SwitchProps extends React.ComponentPropsWithoutRef<typeof SwitchPrimitive.Root> {
  checked?: boolean;
  onCheckedChange?: (checked: boolean) => void;
}

const Switch = React.forwardRef<React.ElementRef<typeof SwitchPrimitive.Root>, SwitchProps>(
  ({ className, checked, onCheckedChange, ...props }, ref) => (
    <SwitchPrimitive.Root
      ref={ref}
      className={cn(
        "relative inline-flex h-[20px] w-[40px] cursor-pointer rounded-full border border-gray-300 bg-gray-200 transition-colors focus:outline-none focus:ring-2 focus:ring-blue-500",
        className
      )}
      checked={checked}
      onCheckedChange={onCheckedChange}
      {...props}
    >
      <SwitchPrimitive.Thumb
        className={cn(
          "block h-[16px] w-[16px] transform rounded-full bg-white shadow transition-transform duration-200",
          checked ? "translate-x-[20px]" : "translate-x-[2px]"
        )}
      />
    </SwitchPrimitive.Root>
  )
);

Switch.displayName = "Switch";

export { Switch };