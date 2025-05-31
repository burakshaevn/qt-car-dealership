-- Add notification_shown column to service_requests
ALTER TABLE public.service_requests 
ADD COLUMN IF NOT EXISTS notification_shown boolean DEFAULT false;

-- Add notification_shown column to insurance_requests
ALTER TABLE public.insurance_requests 
ADD COLUMN IF NOT EXISTS notification_shown boolean DEFAULT false;

-- Add notification_shown column to loan_requests
ALTER TABLE public.loan_requests 
ADD COLUMN IF NOT EXISTS notification_shown boolean DEFAULT false;

-- Add notification_shown column to sell_requests
ALTER TABLE public.sell_requests 
ADD COLUMN IF NOT EXISTS notification_shown boolean DEFAULT false; 