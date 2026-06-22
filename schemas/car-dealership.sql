--
-- PostgreSQL database dump
--

\restrict xexo4BGoEvzPJg1Esljmp3rfvxzHMjs13rrqjnprKWvpqCwHOcfFEHS1NhukuFi

-- Dumped from database version 18.0
-- Dumped by pg_dump version 18.0

-- Started on 2026-06-22 21:28:44

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- TOC entry 245 (class 1255 OID 16385)
-- Name: handle_approved_insurance_request(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.handle_approved_insurance_request() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF NEW.status = 'одобрено' AND OLD.status != 'одобрено' THEN
        INSERT INTO purchases (car_id, client_id, тип_оплаты, тип_страховки)
        VALUES (NEW.car_id, NEW.client_id, 'наличные', NEW.insurance_type);
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.handle_approved_insurance_request() OWNER TO postgres;

--
-- TOC entry 246 (class 1255 OID 25125)
-- Name: handle_approved_loan_request(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.handle_approved_loan_request() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Исправлено: было 'approved', стало 'одобрено'
    IF NEW.status = 'одобрено' AND OLD.status != 'одобрено' THEN
        INSERT INTO purchases (car_id, client_id, тип_оплаты, сумма_кредита, срок_кредита_месяцев)
        VALUES (NEW.car_id, NEW.client_id, 'кредит', NEW.loan_amount, NEW.loan_term_months);
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.handle_approved_loan_request() OWNER TO postgres;

--
-- TOC entry 247 (class 1255 OID 16387)
-- Name: handle_approved_order_request(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.handle_approved_order_request() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF NEW.status = 'одобрено' AND OLD.status <> 'одобрено' THEN
        -- Находим car_id по имени автомобиля
        DECLARE
            target_car_id integer;
        BEGIN
            SELECT id INTO target_car_id FROM public.cars WHERE name = NEW.car_name LIMIT 1;
            IF target_car_id IS NOT NULL THEN
                -- Создаем покупку
                INSERT INTO public.purchases (car_id, client_id, тип_оплаты)
                VALUES (target_car_id, NEW.client_id, 'наличные');
            END IF;
        END;
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.handle_approved_order_request() OWNER TO postgres;

--
-- TOC entry 248 (class 1255 OID 16388)
-- Name: handle_approved_purchase_request(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.handle_approved_purchase_request() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF NEW.status = 'одобрено' AND OLD.status <> 'одобрено' THEN
        -- Проверим доступность на складе
        PERFORM 1 FROM public.cars WHERE id = NEW.car_id AND stock_qty > 0;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Нельзя одобрить заявку: нет автомобиля на складе (car_id=%).', NEW.car_id;
        END IF;
        
        -- Списываем 1 шт. со склада
        UPDATE public.cars
           SET stock_qty = stock_qty - 1
         WHERE id = NEW.car_id;
        
        -- Создаём покупку
        INSERT INTO public.purchases (car_id, client_id, тип_оплаты)
        VALUES (NEW.car_id, NEW.client_id, 'наличные');
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.handle_approved_purchase_request() OWNER TO postgres;

--
-- TOC entry 249 (class 1255 OID 16389)
-- Name: mark_notification_as_viewed(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.mark_notification_as_viewed() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Р•СЃР»Рё СѓРІРµРґРѕРјР»РµРЅРёРµ РїСЂРѕСЃРјР°С‚СЂРёРІР°РµС‚СЃСЏ, СѓСЃС‚Р°РЅР°РІР»РёРІР°РµРј С„Р»Р°Рі
    IF OLD.notification_shown = false AND NEW.notification_shown = true THEN
        -- РќРёС‡РµРіРѕ РЅРµ РґРµР»Р°РµРј, РїСЂРѕСЃС‚Рѕ РїРѕРјРµС‡Р°РµРј РєР°Рє РїСЂРѕСЃРјРѕС‚СЂРµРЅРЅРѕРµ
        RETURN NEW;
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.mark_notification_as_viewed() OWNER TO postgres;

--
-- TOC entry 250 (class 1255 OID 16390)
-- Name: reset_notification_on_status_change(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.reset_notification_on_status_change() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Р•СЃР»Рё СЃС‚Р°С‚СѓСЃ РёР·РјРµРЅРёР»СЃСЏ, СЃР±СЂР°СЃС‹РІР°РµРј С„Р»Р°Рі СѓРІРµРґРѕРјР»РµРЅРёСЏ
    IF OLD.status != NEW.status THEN
        NEW.notification_shown = false;
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.reset_notification_on_status_change() OWNER TO postgres;

--
-- TOC entry 251 (class 1255 OID 16391)
-- Name: update_notification_status(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.update_notification_status() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Обновляем статус уведомления на просмотренное
    NEW.notification_shown = true;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.update_notification_status() OWNER TO postgres;

--
-- TOC entry 252 (class 1255 OID 16392)
-- Name: validate_purchase_availability(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.validate_purchase_availability() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Проверяем, что автомобиль есть на складе
    IF NOT EXISTS (
        SELECT 1 FROM public.cars 
        WHERE id = NEW.car_id 
        AND stock_qty > 0
    ) THEN
        RAISE EXCEPTION 'Автомобиль с ID % недоступен для покупки (нет в наличии)', NEW.car_id;
    END IF;
    
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.validate_purchase_availability() OWNER TO postgres;

--
-- TOC entry 253 (class 1255 OID 16393)
-- Name: validate_rental_availability(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.validate_rental_availability() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Проверяем, что автомобиль доступен для аренды
    IF NOT EXISTS (
        SELECT 1 FROM public.cars 
        WHERE id = NEW.car_id 
        AND available_for_rent = true 
        AND stock_qty > 0
    ) THEN
        RAISE EXCEPTION 'Автомобиль с ID % недоступен для аренды (нет в наличии или не доступен для аренды)', NEW.car_id;
    END IF;
    
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.validate_rental_availability() OWNER TO postgres;

--
-- TOC entry 254 (class 1255 OID 16394)
-- Name: validate_test_drive_availability(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.validate_test_drive_availability() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Проверяем, что автомобиль доступен для тест-драйва
    IF NOT EXISTS (
        SELECT 1 FROM public.cars 
        WHERE id = NEW.car_id 
        AND stock_qty > 0
    ) THEN
        RAISE EXCEPTION 'Автомобиль с ID % недоступен для тест-драйва (нет в наличии)', NEW.car_id;
    END IF;
    
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.validate_test_drive_availability() OWNER TO postgres;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- TOC entry 219 (class 1259 OID 16395)
-- Name: admins; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.admins (
    id integer NOT NULL,
    username character varying(100) NOT NULL,
    password text NOT NULL
);


ALTER TABLE public.admins OWNER TO postgres;

--
-- TOC entry 5226 (class 0 OID 0)
-- Dependencies: 219
-- Name: TABLE admins; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.admins IS 'Администраторы системы';


--
-- TOC entry 220 (class 1259 OID 16403)
-- Name: admins_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.admins_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.admins_id_seq OWNER TO postgres;

--
-- TOC entry 5227 (class 0 OID 0)
-- Dependencies: 220
-- Name: admins_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.admins_id_seq OWNED BY public.admins.id;


--
-- TOC entry 221 (class 1259 OID 16404)
-- Name: car_types; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.car_types (
    id integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.car_types OWNER TO postgres;

--
-- TOC entry 5228 (class 0 OID 0)
-- Dependencies: 221
-- Name: TABLE car_types; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.car_types IS 'Типы автомобилей';


--
-- TOC entry 222 (class 1259 OID 16411)
-- Name: car_types_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.car_types_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.car_types_id_seq OWNER TO postgres;

--
-- TOC entry 5229 (class 0 OID 0)
-- Dependencies: 222
-- Name: car_types_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.car_types_id_seq OWNED BY public.car_types.id;


--
-- TOC entry 223 (class 1259 OID 16412)
-- Name: cars; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.cars (
    id integer NOT NULL,
    name character varying(255) NOT NULL,
    color character varying(50) NOT NULL,
    price numeric(15,0) NOT NULL,
    description text,
    image_url text,
    type_id integer NOT NULL,
    available_for_rent boolean DEFAULT true NOT NULL,
    "trim" character varying(100),
    stock_qty integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.cars OWNER TO postgres;

--
-- TOC entry 5230 (class 0 OID 0)
-- Dependencies: 223
-- Name: TABLE cars; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.cars IS 'Автомобили в наличии';


--
-- TOC entry 224 (class 1259 OID 16426)
-- Name: cars_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.cars_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.cars_id_seq OWNER TO postgres;

--
-- TOC entry 5231 (class 0 OID 0)
-- Dependencies: 224
-- Name: cars_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.cars_id_seq OWNED BY public.cars.id;


--
-- TOC entry 225 (class 1259 OID 16427)
-- Name: clients; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.clients (
    id integer NOT NULL,
    first_name character varying(100) NOT NULL,
    last_name character varying(100) NOT NULL,
    phone character varying(15) NOT NULL,
    email character varying(255) NOT NULL,
    password text NOT NULL
);


ALTER TABLE public.clients OWNER TO postgres;

--
-- TOC entry 5232 (class 0 OID 0)
-- Dependencies: 225
-- Name: TABLE clients; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.clients IS 'Клиенты автосалона';


--
-- TOC entry 226 (class 1259 OID 16438)
-- Name: clients_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.clients_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.clients_id_seq OWNER TO postgres;

--
-- TOC entry 5233 (class 0 OID 0)
-- Dependencies: 226
-- Name: clients_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.clients_id_seq OWNED BY public.clients.id;


--
-- TOC entry 244 (class 1259 OID 115182)
-- Name: contract_templates; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.contract_templates (
    id bigint NOT NULL,
    code character varying(50) NOT NULL,
    title character varying(255) NOT NULL,
    body_template text NOT NULL,
    version integer DEFAULT 1 NOT NULL,
    is_active boolean DEFAULT true NOT NULL,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP NOT NULL
);


ALTER TABLE public.contract_templates OWNER TO postgres;

--
-- TOC entry 243 (class 1259 OID 115181)
-- Name: contract_templates_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.contract_templates ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.contract_templates_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 227 (class 1259 OID 16439)
-- Name: insurance_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.insurance_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    insurance_type character varying(50) NOT NULL,
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    notification_shown boolean DEFAULT false,
    CONSTRAINT insurance_requests_insurance_type_check CHECK (((insurance_type)::text = ANY (ARRAY[('ОСАГО'::character varying)::text, ('КАСКО'::character varying)::text, ('Комплекс'::character varying)::text]))),
    CONSTRAINT insurance_requests_status_check CHECK (((status)::text = ANY (ARRAY[('не обработано'::character varying)::text, ('одобрено'::character varying)::text, ('отклонено'::character varying)::text, ('завершено'::character varying)::text])))
);


ALTER TABLE public.insurance_requests OWNER TO postgres;

--
-- TOC entry 5234 (class 0 OID 0)
-- Dependencies: 227
-- Name: TABLE insurance_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.insurance_requests IS 'Заявки на страхование';


--
-- TOC entry 228 (class 1259 OID 16452)
-- Name: insurance_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.insurance_requests_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.insurance_requests_id_seq OWNER TO postgres;

--
-- TOC entry 5235 (class 0 OID 0)
-- Dependencies: 228
-- Name: insurance_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.insurance_requests_id_seq OWNED BY public.insurance_requests.id;


--
-- TOC entry 229 (class 1259 OID 16453)
-- Name: loan_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.loan_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    loan_amount numeric(15,0) NOT NULL,
    loan_term_months integer NOT NULL,
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    notification_shown boolean DEFAULT false,
    CONSTRAINT loan_requests_loan_amount_check CHECK ((loan_amount > (0)::numeric)),
    CONSTRAINT loan_requests_loan_term_months_check CHECK ((loan_term_months > 0)),
    CONSTRAINT loan_requests_status_check CHECK (((status)::text = ANY (ARRAY[('не обработано'::character varying)::text, ('одобрено'::character varying)::text, ('отклонено'::character varying)::text, ('завершено'::character varying)::text])))
);


ALTER TABLE public.loan_requests OWNER TO postgres;

--
-- TOC entry 5236 (class 0 OID 0)
-- Dependencies: 229
-- Name: TABLE loan_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.loan_requests IS 'Заявки на кредитование';


--
-- TOC entry 230 (class 1259 OID 16468)
-- Name: loan_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.loan_requests_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.loan_requests_id_seq OWNER TO postgres;

--
-- TOC entry 5237 (class 0 OID 0)
-- Dependencies: 230
-- Name: loan_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.loan_requests_id_seq OWNED BY public.loan_requests.id;


--
-- TOC entry 231 (class 1259 OID 16469)
-- Name: order_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.order_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_name character varying(255) NOT NULL,
    color character varying(50),
    "trim" character varying(100),
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    notification_shown boolean DEFAULT false
);


ALTER TABLE public.order_requests OWNER TO postgres;

--
-- TOC entry 232 (class 1259 OID 16479)
-- Name: order_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.order_requests ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.order_requests_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 233 (class 1259 OID 16480)
-- Name: purchase_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.purchase_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    notification_shown boolean DEFAULT false,
    CONSTRAINT purchase_requests_status_check CHECK (((status)::text = ANY (ARRAY[('не обработано'::character varying)::text, ('одобрено'::character varying)::text, ('отклонено'::character varying)::text, ('завершено'::character varying)::text])))
);


ALTER TABLE public.purchase_requests OWNER TO postgres;

--
-- TOC entry 234 (class 1259 OID 16491)
-- Name: purchase_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.purchase_requests ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.purchase_requests_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 235 (class 1259 OID 16492)
-- Name: purchases; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.purchases (
    id integer NOT NULL,
    car_id integer NOT NULL,
    client_id integer NOT NULL,
    purchase_date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    "тип_оплаты" character varying(20) DEFAULT 'наличные'::character varying NOT NULL,
    "сумма_кредита" numeric(15,0),
    "срок_кредита_месяцев" integer,
    "тип_страховки" character varying(50),
    CONSTRAINT "проверка_срока_кредита" CHECK (("срок_кредита_месяцев" > 0)),
    CONSTRAINT "проверка_суммы_кредита" CHECK (("сумма_кредита" > (0)::numeric)),
    CONSTRAINT "проверка_типа_оплаты" CHECK ((("тип_оплаты")::text = ANY (ARRAY[('наличные'::character varying)::text, ('кредит'::character varying)::text]))),
    CONSTRAINT "проверка_типа_страховки" CHECK ((("тип_страховки")::text = ANY (ARRAY[('ОСАГО'::character varying)::text, ('КАСКО'::character varying)::text, ('Комплекс'::character varying)::text])))
);


ALTER TABLE public.purchases OWNER TO postgres;

--
-- TOC entry 5238 (class 0 OID 0)
-- Dependencies: 235
-- Name: TABLE purchases; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.purchases IS 'История покупок автомобилей';


--
-- TOC entry 236 (class 1259 OID 16505)
-- Name: purchases_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.purchases_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.purchases_id_seq OWNER TO postgres;

--
-- TOC entry 5239 (class 0 OID 0)
-- Dependencies: 236
-- Name: purchases_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.purchases_id_seq OWNED BY public.purchases.id;


--
-- TOC entry 237 (class 1259 OID 16506)
-- Name: rental_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.rental_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    rental_days integer NOT NULL,
    start_date date NOT NULL,
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    notification_shown boolean DEFAULT false,
    CONSTRAINT rental_requests_rental_days_check CHECK ((rental_days > 0)),
    CONSTRAINT rental_requests_status_check CHECK (((status)::text = ANY (ARRAY[('не обработано'::character varying)::text, ('одобрено'::character varying)::text, ('отклонено'::character varying)::text, ('завершено'::character varying)::text])))
);


ALTER TABLE public.rental_requests OWNER TO postgres;

--
-- TOC entry 5240 (class 0 OID 0)
-- Dependencies: 237
-- Name: TABLE rental_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.rental_requests IS 'Заявки на аренду автомобилей';


--
-- TOC entry 238 (class 1259 OID 16520)
-- Name: rental_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.rental_requests ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.rental_requests_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 239 (class 1259 OID 16521)
-- Name: service_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.service_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    service_type character varying(100) NOT NULL,
    scheduled_date timestamp without time zone NOT NULL,
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    notification_shown boolean DEFAULT false,
    CONSTRAINT service_requests_status_check CHECK (((status)::text = ANY (ARRAY[('не обработано'::character varying)::text, ('подтверждено'::character varying)::text, ('выполнено'::character varying)::text, ('отменено'::character varying)::text])))
);


ALTER TABLE public.service_requests OWNER TO postgres;

--
-- TOC entry 5241 (class 0 OID 0)
-- Dependencies: 239
-- Name: TABLE service_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.service_requests IS 'Заявки на сервисное обслуживание';


--
-- TOC entry 240 (class 1259 OID 16534)
-- Name: service_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.service_requests_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.service_requests_id_seq OWNER TO postgres;

--
-- TOC entry 5242 (class 0 OID 0)
-- Dependencies: 240
-- Name: service_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.service_requests_id_seq OWNED BY public.service_requests.id;


--
-- TOC entry 241 (class 1259 OID 16535)
-- Name: test_drives; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.test_drives (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    scheduled_date timestamp without time zone NOT NULL,
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    notification_shown boolean DEFAULT false,
    CONSTRAINT test_drives_status_check CHECK (((status)::text = ANY (ARRAY[('не обработано'::character varying)::text, ('одобрено'::character varying)::text, ('отклонено'::character varying)::text])))
);


ALTER TABLE public.test_drives OWNER TO postgres;

--
-- TOC entry 5243 (class 0 OID 0)
-- Dependencies: 241
-- Name: TABLE test_drives; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.test_drives IS 'Заявки на тест-драйв';


--
-- TOC entry 242 (class 1259 OID 16547)
-- Name: test_drives_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.test_drives_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.test_drives_id_seq OWNER TO postgres;

--
-- TOC entry 5244 (class 0 OID 0)
-- Dependencies: 242
-- Name: test_drives_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.test_drives_id_seq OWNED BY public.test_drives.id;


--
-- TOC entry 4926 (class 2604 OID 16548)
-- Name: admins id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins ALTER COLUMN id SET DEFAULT nextval('public.admins_id_seq'::regclass);


--
-- TOC entry 4927 (class 2604 OID 16549)
-- Name: car_types id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types ALTER COLUMN id SET DEFAULT nextval('public.car_types_id_seq'::regclass);


--
-- TOC entry 4928 (class 2604 OID 16550)
-- Name: cars id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars ALTER COLUMN id SET DEFAULT nextval('public.cars_id_seq'::regclass);


--
-- TOC entry 4931 (class 2604 OID 16551)
-- Name: clients id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients ALTER COLUMN id SET DEFAULT nextval('public.clients_id_seq'::regclass);


--
-- TOC entry 4932 (class 2604 OID 16552)
-- Name: insurance_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests ALTER COLUMN id SET DEFAULT nextval('public.insurance_requests_id_seq'::regclass);


--
-- TOC entry 4936 (class 2604 OID 16553)
-- Name: loan_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests ALTER COLUMN id SET DEFAULT nextval('public.loan_requests_id_seq'::regclass);


--
-- TOC entry 4946 (class 2604 OID 16554)
-- Name: purchases id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases ALTER COLUMN id SET DEFAULT nextval('public.purchases_id_seq'::regclass);


--
-- TOC entry 4952 (class 2604 OID 16555)
-- Name: service_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests ALTER COLUMN id SET DEFAULT nextval('public.service_requests_id_seq'::regclass);


--
-- TOC entry 4956 (class 2604 OID 16556)
-- Name: test_drives id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives ALTER COLUMN id SET DEFAULT nextval('public.test_drives_id_seq'::regclass);


--
-- TOC entry 5195 (class 0 OID 16395)
-- Dependencies: 219
-- Data for Name: admins; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.admins VALUES (2, 'admin2', 'adminpass2');
INSERT INTO public.admins VALUES (3, 'admin3', 'adminpass3');
INSERT INTO public.admins VALUES (1, 'admin1', 'password123');


--
-- TOC entry 5197 (class 0 OID 16404)
-- Dependencies: 221
-- Data for Name: car_types; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.car_types VALUES (2, 'Внедорожник');
INSERT INTO public.car_types VALUES (3, 'Купе');
INSERT INTO public.car_types VALUES (4, 'Кабриолет');
INSERT INTO public.car_types VALUES (1, 'Лимузин');


--
-- TOC entry 5199 (class 0 OID 16412)
-- Dependencies: 223
-- Data for Name: cars; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.cars VALUES (5, 'S 63 E Performance', 'Золотой магнолитет', 20520000, 'Роскошный седан в эксклюзивном золотом цвете. Мощный гибридный двигатель и инновационные технологии.', 'Mercedes-AMG S 63 E Performance\kalaharigold magno.png', 1, true, 'Премиум', 2);
INSERT INTO public.cars VALUES (7, 'S 63 E Performance', 'Чёрный', 20520000, 'Стильный чёрный седан с матовым покрытием. Впечатляющая мощность и динамичные характеристики.', 'Mercedes-AMG S 63 E Performance\nachtschwarz magno.png', 1, true, 'Эксклюзив', 2);
INSERT INTO public.cars VALUES (10, 'S 63 E Performance', 'Голубой', 20520000, 'Эксклюзивный седан в винтажно-голубом цвете. Сочетание классического стиля и современных технологий.', 'Mercedes-AMG S 63 E Performance\vintageblau uni.png', 1, true, 'Премиум', 2);
INSERT INTO public.cars VALUES (15, 'Mercedes-AMG G 63', 'Бриллиант', 19890000, 'Легендарный внедорожник в бриллиантово-синем цвете. Непревзойдённая проходимость и мощный двигатель.', 'Mercedes-AMG G 63\blue.png', 2, true, 'AMG', 2);
INSERT INTO public.cars VALUES (17, 'Mercedes-AMG G 63', 'Серый', 19890000, 'Серый внедорожник с брутальным дизайном. Отличная управляемость на любом покрытии.', 'Mercedes-AMG G 63\gray.png', 2, true, 'AMG', 1);
INSERT INTO public.cars VALUES (18, 'Mercedes-AMG G 63', 'Зелёный', 19890000, 'Зелёный внедорожник для ценителей стиля. Мощный двигатель и комфортабельный интерьер.', 'Mercedes-AMG G 63\green.png', 2, true, 'AMG', 2);
INSERT INTO public.cars VALUES (19, 'Mercedes-AMG G 63', 'Красный', 19890000, 'Яркий красный внедорожник с агрессивным дизайном. Впечатляющая динамика и проходимость.', 'Mercedes-AMG G 63\red.png', 2, true, 'AMG', 1);
INSERT INTO public.cars VALUES (20, 'Mercedes-AMG G 63', 'Жёлтый', 19890000, 'Жёлтый внедорожник для смелых водителей. Уникальный внешний вид и выдающиеся характеристики.', 'Mercedes-AMG G 63\yellow.png', 2, true, 'AMG', 1);
INSERT INTO public.cars VALUES (21, 'Mercedes-AMG GLB 35 4MATIC', 'Чёрный', 6840000, 'Компактный кроссовер в чёрном цвете. Универсальный автомобиль для города и путешествий.', 'Mercedes-AMG GLB 35 4MATIC\black.png', 2, true, 'AMG Line', 4);
INSERT INTO public.cars VALUES (22, 'Mercedes-AMG GLB 35 4MATIC', 'Синий', 6840000, 'Стильный синий кроссовер с динамичным характером. Вместительный салон и экономичный двигатель.', 'Mercedes-AMG GLB 35 4MATIC\blue.png', 2, true, 'AMG Line', 3);
INSERT INTO public.cars VALUES (23, 'Mercedes-AMG GLB 35 4MATIC', 'Красный', 6840000, 'Энергичный красный кроссовер. Отличная управляемость и современные технологии.', 'Mercedes-AMG GLB 35 4MATIC\red.png', 2, true, 'AMG Line', 2);
INSERT INTO public.cars VALUES (24, 'Mercedes-AMG GLB 35 4MATIC', 'Белый', 6840000, 'Белый кроссовер с элегантным дизайном. Комфорт и практичность в одном автомобиле.', 'Mercedes-AMG GLB 35 4MATIC\white.png', 2, true, 'AMG Line', 3);
INSERT INTO public.cars VALUES (25, 'GLC 43 4MATIC Coupé', 'Чёрный', 8840000, 'Спортивный купе-кроссовер в чёрном цвете. Динамичный дизайн и мощный двигатель.', 'Mercedes-AMG GLC 43 4MATIC Coupé\black.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (27, 'GLC 43 4MATIC Coupé', 'Красный', 8840000, 'Яркий красный купе-кроссовер. Спортивный характер и премиальное оснащение.', 'Mercedes-AMG GLC 43 4MATIC Coupé\red.png', 3, true, 'AMG', 1);
INSERT INTO public.cars VALUES (28, 'GLC 43 4MATIC Coupé', 'Белый', 8840000, 'Белоснежный купе-кроссовер с роскошным интерьером. Комфорт и динамика в одном автомобиле.', 'Mercedes-AMG GLC 43 4MATIC Coupé\white.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (38, 'GLS 63 4MATIC+', 'Чёрный', 18495000, 'Большой чёрный внедорожник премиум-класса. Просторный салон и мощный двигатель.', 'Mercedes-AMG GLS 63 4MATIC+\black.png', 2, true, 'AMG', 2);
INSERT INTO public.cars VALUES (39, 'GLS 63 4MATIC+', 'Синий', 18495000, 'Голубой внедорожник с представительским характером. Высокий уровень комфорта и безопасности.', 'Mercedes-AMG GLS 63 4MATIC+\blue.png', 2, true, 'AMG', 3);
INSERT INTO public.cars VALUES (40, 'GLS 63 4MATIC+', 'Зелёный', 18495000, 'Зелёный внедорожник для любителей природы. Вместительный и технологичный автомобиль.', 'Mercedes-AMG GLS 63 4MATIC+\green.png', 2, true, 'AMG', 1);
INSERT INTO public.cars VALUES (41, 'GLS 63 4MATIC+', 'Красный', 18495000, 'Красный внедорожник с ярким характером. Мощность и роскошь в одном автомобиле.', 'Mercedes-AMG GLS 63 4MATIC+\red.png', 2, true, 'AMG', 2);
INSERT INTO public.cars VALUES (42, 'GLS 63 4MATIC+', 'Серый', 18495000, 'Серый внедорожник с элегантным дизайном. Идеальный выбор для семьи и путешествий.', 'Mercedes-AMG GLS 63 4MATIC+\gray.png', 2, true, 'AMG', 1);
INSERT INTO public.cars VALUES (43, 'Mercedes-AMG SL 43', 'Чёрный', 5563000, 'Роскошный чёрный родстер. Идеальный автомобиль для любителей открытых дорог.', 'Mercedes-AMG SL 43\black.png', 4, true, 'AMG', 3);
INSERT INTO public.cars VALUES (44, 'Mercedes-AMG SL 43', 'Голубой', 5563000, 'Элегантный голубой родстер. Сочетание стиля и выдающихся динамических характеристик.', 'Mercedes-AMG SL 43\blue.png', 4, true, 'AMG', 2);
INSERT INTO public.cars VALUES (45, 'Mercedes-AMG SL 43', 'Красный', 5563000, 'Страстный красный родстер. Спортивный характер и неповторимый дизайн.', 'Mercedes-AMG SL 43\red.png', 4, true, 'AMG', 1);
INSERT INTO public.cars VALUES (46, 'Mercedes-AMG SL 43', 'Белый', 5563000, 'Белоснежный родстер с премиальным интерьером. Комфорт и удовольствие от вождения.', 'Mercedes-AMG SL 43\white.png', 4, true, 'AMG', 2);
INSERT INTO public.cars VALUES (47, 'Mercedes-AMG SL 43', 'Жёлтый', 5563000, 'Яркий жёлтый родстер для смелых водителей. Динамичный дизайн и мощный двигатель.', 'Mercedes-AMG SL 43\yellow.png', 4, true, 'AMG', 1);
INSERT INTO public.cars VALUES (48, 'Mercedes-AMG GT 43', 'Чёрный', 12209000, 'Спортивный чёрный седан. Мощный двигатель и агрессивный дизайн.', 'Mercedes-AMG GT 43\black.png', 3, true, 'AMG', 3);
INSERT INTO public.cars VALUES (49, 'Mercedes-AMG GT 43', 'Серый', 12209000, 'Элегантный серый седан с динамичным характером. Премиальное оснащение и комфорт.', 'Mercedes-AMG GT 43\gray.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (50, 'Mercedes-AMG GT 43', 'Зелёный', 12209000, 'Эксклюзивный зелёный седан. Уникальный внешний вид и выдающиеся характеристики.', 'Mercedes-AMG GT 43\green.png', 3, true, 'AMG', 1);
INSERT INTO public.cars VALUES (51, 'Mercedes-AMG GT 43', 'Голубой', 12209000, 'Яркий голубой седан с футуристичным дизайном. Современные технологии и динамика.', 'Mercedes-AMG GT 43\hyperblau.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (52, 'Mercedes-AMG GT 43', 'Красный', 12209000, 'Страстный красный седан. Спортивный характер и роскошный интерьер.', 'Mercedes-AMG GT 43\red.png', 3, true, 'AMG', 3);
INSERT INTO public.cars VALUES (53, 'Mercedes-AMG GT 43', 'Белый', 12209000, 'Белоснежный седан премиум-класса. Сочетание элегантности и мощности.', 'Mercedes-AMG GT 43\white.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (59, 'GT 63 S 4MATIC+', 'Чёрный', 8420000, 'Агрессивный чёрный спортивный автомобиль. Мощный двигатель и выдающаяся динамика.', 'Mercedes-AMG GT 63 S 4MATIC+\black.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (60, 'GT 63 S 4MATIC+', 'Зелёный', 8420000, 'Эксклюзивный зелёный спортивный автомобиль. Уникальный дизайн и премиальное оснащение.', 'Mercedes-AMG GT 63 S 4MATIC+\green.png', 3, true, 'AMG', 1);
INSERT INTO public.cars VALUES (61, 'GT 63 S 4MATIC+', 'Красный', 8420000, 'Страстный красный спортивный автомобиль. Идеальный выбор для любителей скорости.', 'Mercedes-AMG GT 63 S 4MATIC+\red.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (62, 'GT 63 S 4MATIC+', 'Белый', 8420000, 'Элегантный белый спортивный автомобиль. Сочетание стиля и мощности.', 'Mercedes-AMG GT 63 S 4MATIC+\white.png', 3, true, 'AMG', 3);
INSERT INTO public.cars VALUES (63, 'GT 63 S 4MATIC+', 'Жёлтый', 8420000, 'Яркий жёлтый спортивный автомобиль. Неповторимый внешний вид и динамичные характеристики.', 'Mercedes-AMG GT 63 S 4MATIC+\yellow.png', 3, true, 'AMG', 1);
INSERT INTO public.cars VALUES (64, 'CLE 53 4MATIC+ Coupé', 'Чёрный', 9051000, 'Стильный чёрный купе. Динамичный дизайн и мощный двигатель.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\black.png', 3, true, 'AMG', 2);
INSERT INTO public.cars VALUES (65, 'CLE 53 4MATIC+ Coupé', 'Красный', 9051000, 'Энергичный красный купе. Спортивный характер и современные технологии.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\red.png', 3, true, 'AMG', 3);
INSERT INTO public.cars VALUES (68, 'GLS 63 4MATIC+', 'Белый', 18495000, 'Белый внедорожник премиум-класса. Просторный салон и передовые технологии.', 'Mercedes-AMG GLS 63 4MATIC+\white.png', 2, true, 'AMG', 3);
INSERT INTO public.cars VALUES (71, 'E 220 d Limousine', 'Красный', 6077570, 'Динамичный красный седан E-класса. Спортивный характер и отличная управляемость.', 'E 220 d Limousine\red.png', 1, true, 'AMG Line', 2);
INSERT INTO public.cars VALUES (6, 'S 63 E Performance', 'Синий', 20520000, 'Изысканный седан в мистическом синем цвете. Превосходная управляемость и максимальный комфорт.', 'Mercedes-AMG S 63 E Performance\mysticblau metallic.png', 1, true, 'Премиум', 1);
INSERT INTO public.cars VALUES (9, 'S 63 E Performance', 'Белый', 20520000, 'Белоснежный представительский седан. Просторный салон с роскошной отделкой и передовыми системами.', 'Mercedes-AMG S 63 E Performance\opalithwei bright.png', 1, true, 'Премиум', 3);
INSERT INTO public.cars VALUES (12, 'EQS 53 4MATIC+', 'Белый', 15700000, 'Футуристичный электромобиль в белом цвете. Инновационный дизайн и выдающаяся дальность хода.', 'Mercedes-AMG EQS 53 4MATIC+\white.png', 1, true, 'AMG Line', 2);
INSERT INTO public.cars VALUES (4, 'CLE 200 Cabriolet', 'Белый', 5683000, 'Белоснежный кабриолет с роскошным интерьером. Современные технологии и высокий уровень комфорта.', 'CLE 200 Cabriolet\white.png', 4, true, 'Люкс', 0);
INSERT INTO public.cars VALUES (70, 'E 220 d Limousine', 'Чёрный', 6077570, 'Стильный чёрный седан E-класса. Премиальная отделка и современные технологии.', 'E 220 d Limousine\black.png', 1, true, 'Avantgarde', 3);
INSERT INTO public.cars VALUES (13, 'EQS 53 4MATIC+', 'Красный', 15700000, 'Динамичный электрокар в красном цвете. Мгновенный разгон и премиальное оснащение.', 'Mercedes-AMG EQS 53 4MATIC+\red.png', 1, true, 'AMG Line', 2);
INSERT INTO public.cars VALUES (14, 'EQS 53 4MATIC+', 'Чёрный', 15700000, 'Элегантный чёрный электромобиль. Тихая и плавная езда с высоким уровнем комфорта.', 'Mercedes-AMG EQS 53 4MATIC+\black.png', 1, true, 'AMG Line', 3);
INSERT INTO public.cars VALUES (2, 'CLE 200 Cabriolet', 'Синий', 5683000, 'Стильный кабриолет в голубом цвете. Отличная динамика и топливная экономичность.', 'CLE 200 Cabriolet\blue.png', 4, true, 'Базовый', 0);
INSERT INTO public.cars VALUES (3, 'CLE 200 Cabriolet', 'Красный', 5683000, 'Яркий красный кабриолет с спортивным характером. Вместительный салон и премиальная отделка.', 'CLE 200 Cabriolet\red.png', 4, true, 'Спорт', 0);
INSERT INTO public.cars VALUES (26, 'GLC 43 4MATIC Coupé', 'Синий', 8840000, 'Элегантный голубой купе-кроссовер. Сочетание стиля и выдающихся ходовых качеств.', 'Mercedes-AMG GLC 43 4MATIC Coupé\blue.png', 3, true, 'AMG', 3);
INSERT INTO public.cars VALUES (67, 'CLE 53 4MATIC+ Coupé', 'Жёлтый', 9051000, 'Яркий жёлтый купе для смелых водителей. Уникальный внешний вид и выдающаяся динамика.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\yellow.png', 3, true, 'AMG', 1);
INSERT INTO public.cars VALUES (66, 'CLE 53 4MATIC+ Coupé', 'Белый', 9051000, 'Элегантный белый купе. Роскошный интерьер и комфортабельный салон.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\white.png', 3, true, 'AMG', 0);
INSERT INTO public.cars VALUES (69, 'E 220 d Limousine', 'Белый', 6077570, 'Элегантный седан E-класса в белом цвете. Экономичный дизельный двигатель и комфортабельный салон.', 'E 220 d Limousine\white.png', 1, true, 'Avantgarde', 4);
INSERT INTO public.cars VALUES (72, 'Mercedes-Maybach SL 680 Monogram Series', 'Красный', 24219570, 'Эксклюзивный красный родстер Maybach. Роскошная отделка и мощный двигатель V12.', 'Mercedes-Maybach SL 680 Monogram Series\red.png', 4, true, 'Maybach', 1);
INSERT INTO public.cars VALUES (73, 'Mercedes-Maybach SL 680 Monogram Series', 'Белый', 24219570, 'Белоснежный родстер Maybach. Максимальный комфорт и премиальное оснащение.', 'Mercedes-Maybach SL 680 Monogram Series\white.png', 4, true, 'Maybach', 1);
INSERT INTO public.cars VALUES (74, 'Mercedes-Maybach SL 680 Monogram Series', 'Чёрный', 24219570, 'Изысканный чёрный родстер Maybach. Элегантность и мощность в одном автомобиле.', 'Mercedes-Maybach SL 680 Monogram Series\black.png', 4, true, 'Maybach', 2);
INSERT INTO public.cars VALUES (75, 'Mercedes-Maybach SL 680 Monogram Series', 'Жёлтый', 24219570, 'Яркий жёлтый родстер Maybach. Уникальный дизайн и выдающиеся характеристики.', 'Mercedes-Maybach SL 680 Monogram Series\yellow.png', 4, true, 'Maybach', 1);
INSERT INTO public.cars VALUES (76, 'Mercedes-Maybach SL 680 Monogram Series', 'Синий', 24219570, 'Элегантный синий родстер Maybach. Сочетание роскоши и спортивного характера.', 'Mercedes-Maybach SL 680 Monogram Series\blue.png', 4, true, 'Maybach', 1);
INSERT INTO public.cars VALUES (16, 'Mercedes-AMG G 63', 'Белый', 19890000, 'Классический белый внедорожник. Просторный салон и передовые системы безопасности.', 'Mercedes-AMG G 63\white.png', 2, true, 'AMG', 2);


--
-- TOC entry 5201 (class 0 OID 16427)
-- Dependencies: 225
-- Data for Name: clients; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.clients VALUES (8, 'Иван', 'Боздунов', '79274800235', 'ivan.bozdunov@mail.ru', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f');
INSERT INTO public.clients VALUES (7, 'Никита', 'Буракшаев', '79274800234', 'nb@example.com', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f');


--
-- TOC entry 5220 (class 0 OID 115182)
-- Dependencies: 244
-- Data for Name: contract_templates; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.contract_templates OVERRIDING SYSTEM VALUE VALUES (1, 'purchase', 'Договор купли-продажи автомобиля', '
<p>Дата заключения: {{current_date}}</p><table>
<tr><td>Автомобиль</td><td>{{car_name}}</td></tr>
<tr><td>Цвет</td><td>{{car_color}}</td></tr>
<tr><td>Стоимость</td><td>{{car_price}} руб.</td></tr></table>
<p>Покупатель подтверждает осмотр автомобиля и согласие с его характеристиками.</p>
<p class="signatures">Продавец: ____________________ Покупатель: ____________________</p>', 1, true, '2026-06-22 21:27:59.114092');
INSERT INTO public.contract_templates OVERRIDING SYSTEM VALUE VALUES (2, 'loan', 'Договор приобретения автомобиля в кредит', '
<p>Дата заключения: {{current_date}}</p><table>
<tr><td>Автомобиль</td><td>{{car_name}}, {{car_color}}</td></tr>
<tr><td>Стоимость</td><td>{{car_price}} руб.</td></tr>
<tr><td>Сумма кредита</td><td>{{loan_amount}} руб.</td></tr>
<tr><td>Срок кредита</td><td>{{loan_term}} мес.</td></tr></table>
<p>Заёмщик обязуется исполнять условия кредитования согласно утверждённому графику платежей.</p>
<p class="signatures">Кредитор: ____________________ Заёмщик: ____________________</p>', 1, true, '2026-06-22 21:27:59.114092');
INSERT INTO public.contract_templates OVERRIDING SYSTEM VALUE VALUES (3, 'rental', 'Договор аренды автомобиля', '
<p>Дата заключения: {{current_date}}</p><table>
<tr><td>Автомобиль</td><td>{{car_name}}, {{car_color}}</td></tr>
<tr><td>Начало аренды</td><td>{{start_date}}</td></tr>
<tr><td>Срок аренды</td><td>{{rental_days}} дней</td></tr></table>
<p>Арендатор принимает автомобиль во временное пользование и обязуется вернуть его в согласованный срок.</p>
<p class="signatures">Арендодатель: ____________________ Арендатор: ____________________</p>', 1, true, '2026-06-22 21:27:59.114092');
INSERT INTO public.contract_templates OVERRIDING SYSTEM VALUE VALUES (4, 'insurance', 'Договор страхования автомобиля', '
<p>Дата заключения: {{current_date}}</p><table>
<tr><td>Автомобиль</td><td>{{car_name}}, {{car_color}}</td></tr>
<tr><td>Программа страхования</td><td>{{insurance_type}}</td></tr></table>
<p>Страховое покрытие действует на условиях выбранной программы и правил страхования.</p>
<p class="signatures">Страховщик: ____________________ Страхователь: ____________________</p>', 1, true, '2026-06-22 21:27:59.114092');
INSERT INTO public.contract_templates OVERRIDING SYSTEM VALUE VALUES (5, 'service', 'Договор сервисного обслуживания', '
<p>Дата заключения: {{current_date}}</p><table>
<tr><td>Автомобиль</td><td>{{car_name}}, {{car_color}}</td></tr>
<tr><td>Вид работ</td><td>{{service_type}}</td></tr>
<tr><td>Дата обслуживания</td><td>{{scheduled_date}}</td></tr></table>
<p>Исполнитель обязуется выполнить согласованные работы, а Заказчик — принять их результат.</p>
<p class="signatures">Исполнитель: ____________________ Заказчик: ____________________</p>', 1, true, '2026-06-22 21:27:59.114092');
INSERT INTO public.contract_templates OVERRIDING SYSTEM VALUE VALUES (6, 'test_drive', 'Соглашение о проведении тест-драйва', '
<p>Дата оформления: {{current_date}}</p><table>
<tr><td>Автомобиль</td><td>{{car_name}}, {{car_color}}</td></tr>
<tr><td>Дата и время тест-драйва</td><td>{{scheduled_date}}</td></tr></table>
<p>Участник принимает ответственность за соблюдение правил дорожного движения и инструкций представителя автосалона.</p>
<p class="signatures">Представитель: ____________________ Участник: ____________________</p>', 1, true, '2026-06-22 21:27:59.114092');
INSERT INTO public.contract_templates OVERRIDING SYSTEM VALUE VALUES (7, 'order', 'Договор заказа автомобиля', '
<p>Дата заключения: {{current_date}}</p><table>
<tr><td>Автомобиль</td><td>{{car_name}}</td></tr>
<tr><td>Цвет</td><td>{{car_color}}</td></tr>
<tr><td>Комплектация</td><td>{{trim}}</td></tr>
<tr><td>Ориентировочная стоимость</td><td>{{car_price}} руб.</td></tr></table>
<p>Продавец принимает заказ на поставку автомобиля с указанными характеристиками.</p>
<p class="signatures">Продавец: ____________________ Заказчик: ____________________</p>', 1, true, '2026-06-22 21:27:59.114092');


--
-- TOC entry 5203 (class 0 OID 16439)
-- Dependencies: 227
-- Data for Name: insurance_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.insurance_requests VALUES (18, 7, 18, 'ОСАГО', 'отклонено', '2025-09-15 06:09:07.575263', true);
INSERT INTO public.insurance_requests VALUES (26, 7, 2, 'ОСАГО', 'одобрено', '2025-09-26 20:01:07.051596', true);
INSERT INTO public.insurance_requests VALUES (28, 8, 39, 'Комплекс', 'одобрено', '2025-10-26 09:59:15.880265', false);
INSERT INTO public.insurance_requests VALUES (27, 7, 38, 'ОСАГО', 'одобрено', '2025-10-25 14:17:32.985435', true);
INSERT INTO public.insurance_requests VALUES (29, 7, 6, 'ОСАГО', 'отклонено', '2025-10-28 12:57:27.210541', true);
INSERT INTO public.insurance_requests VALUES (36, 7, 9, 'Комплекс', 'не обработано', '2026-02-22 11:56:55.428203', true);


--
-- TOC entry 5205 (class 0 OID 16453)
-- Dependencies: 229
-- Data for Name: loan_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.loan_requests VALUES (9, 7, 5, 100006, 5, 'отклонено', '2025-09-13 19:29:17.98794', true);
INSERT INTO public.loan_requests VALUES (10, 7, 68, 18495000, 12, 'одобрено', '2025-09-16 19:27:04.987974', true);
INSERT INTO public.loan_requests VALUES (7, 7, 13, 15700000, 48, 'отклонено', '2025-08-29 08:25:37.135162', true);
INSERT INTO public.loan_requests VALUES (8, 7, 68, 18495000, 12, 'отклонено', '2025-09-13 18:10:57.031649', true);
INSERT INTO public.loan_requests VALUES (14, 7, 2, 100003, 1, 'не обработано', '2025-09-22 20:23:00.567545', true);
INSERT INTO public.loan_requests VALUES (15, 7, 2, 100000, 2, 'не обработано', '2025-09-26 19:50:05.566614', true);
INSERT INTO public.loan_requests VALUES (16, 7, 2, 100000, 2, 'не обработано', '2025-09-26 19:52:50.618505', true);
INSERT INTO public.loan_requests VALUES (13, 7, 2, 100001, 1, 'отклонено', '2025-09-22 20:14:57.012445', true);
INSERT INTO public.loan_requests VALUES (17, 7, 2, 100003, 1, 'отклонено', '2025-09-26 19:56:54.460271', true);
INSERT INTO public.loan_requests VALUES (18, 8, 39, 18495000, 60, 'одобрено', '2025-10-26 09:59:15.875401', false);
INSERT INTO public.loan_requests VALUES (19, 7, 15, 19890000, 12, 'одобрено', '2025-10-27 18:49:11.543256', true);
INSERT INTO public.loan_requests VALUES (21, 7, 6, 20520000, 12, 'одобрено', '2025-10-28 12:57:27.20837', true);
INSERT INTO public.loan_requests VALUES (26, 7, 69, 1000000, 36, 'не обработано', '2026-02-22 12:27:23.485669', true);


--
-- TOC entry 5207 (class 0 OID 16469)
-- Dependencies: 231
-- Data for Name: order_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.order_requests OVERRIDING SYSTEM VALUE VALUES (5, 7, 'CLE 200 Cabriolet', 'Белый', 'Базовый', 'одобрено', '2025-09-16 20:17:55.5038', true);
INSERT INTO public.order_requests OVERRIDING SYSTEM VALUE VALUES (8, 7, 'CLE 53 4MATIC+ Coupé', 'Белый', 'AMG', 'одобрено', '2025-10-28 13:09:37.434893', true);


--
-- TOC entry 5209 (class 0 OID 16480)
-- Dependencies: 233
-- Data for Name: purchase_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.purchase_requests OVERRIDING SYSTEM VALUE VALUES (1, 7, 66, 'одобрено', '2025-09-16 20:37:55.283898', true);
INSERT INTO public.purchase_requests OVERRIDING SYSTEM VALUE VALUES (7, 7, 16, 'одобрено', '2026-02-21 15:43:39.482135', true);
INSERT INTO public.purchase_requests OVERRIDING SYSTEM VALUE VALUES (10, 7, 64, 'не обработано', '2026-02-22 13:06:27.203386', true);


--
-- TOC entry 5211 (class 0 OID 16492)
-- Dependencies: 235
-- Data for Name: purchases; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.purchases VALUES (44, 16, 7, '2026-02-21 15:44:02.887857', 'наличные', NULL, NULL, NULL);


--
-- TOC entry 5213 (class 0 OID 16506)
-- Dependencies: 237
-- Data for Name: rental_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (10, 7, 2, 1, '2025-09-16', 'не обработано', '2025-09-16 20:29:06.670154', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (13, 7, 2, 1, '2025-09-22', 'не обработано', '2025-09-22 20:06:34.154262', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (14, 7, 3, 1, '2025-09-22', 'не обработано', '2025-09-22 20:06:53.483626', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (25, 7, 69, 5, '2025-10-27', 'одобрено', '2025-10-27 16:42:53.345449', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (26, 8, 5, 1, '2025-10-27', 'одобрено', '2025-10-27 18:48:10.300607', false);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (33, 7, 9, 6, '2026-02-22', 'не обработано', '2026-02-22 11:56:55.443955', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (35, 7, 69, 3, '2026-02-22', 'не обработано', '2026-02-22 12:27:15.778696', true);


--
-- TOC entry 5215 (class 0 OID 16521)
-- Dependencies: 239
-- Data for Name: service_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.service_requests VALUES (13, 7, 72, 'Диагностика', '2025-09-23 09:00:00', 'подтверждено', '2025-09-22 19:55:25.371291', true);
INSERT INTO public.service_requests VALUES (15, 7, 2, 'Диагностика', '2025-09-27 09:00:00', 'подтверждено', '2025-09-26 20:00:45.901342', true);


--
-- TOC entry 5217 (class 0 OID 16535)
-- Dependencies: 241
-- Data for Name: test_drives; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.test_drives VALUES (3, 7, 4, '2025-09-14 10:00:00', 'одобрено', '2025-09-13 16:33:32.462598', true);
INSERT INTO public.test_drives VALUES (2, 7, 25, '2025-09-25 09:00:00', 'одобрено', '2025-08-24 07:14:10.668617', true);
INSERT INTO public.test_drives VALUES (4, 7, 40, '2025-09-16 09:00:00', 'одобрено', '2025-09-15 05:25:32.06474', true);
INSERT INTO public.test_drives VALUES (7, 7, 4, '2025-09-17 17:00:00', 'одобрено', '2025-09-16 19:22:48.075726', true);
INSERT INTO public.test_drives VALUES (10, 7, 63, '2025-10-28 09:00:00', 'одобрено', '2025-10-28 12:56:11.990418', true);


--
-- TOC entry 5245 (class 0 OID 0)
-- Dependencies: 220
-- Name: admins_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.admins_id_seq', 3, true);


--
-- TOC entry 5246 (class 0 OID 0)
-- Dependencies: 222
-- Name: car_types_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.car_types_id_seq', 6, true);


--
-- TOC entry 5247 (class 0 OID 0)
-- Dependencies: 224
-- Name: cars_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.cars_id_seq', 81, true);


--
-- TOC entry 5248 (class 0 OID 0)
-- Dependencies: 226
-- Name: clients_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.clients_id_seq', 8, true);


--
-- TOC entry 5249 (class 0 OID 0)
-- Dependencies: 243
-- Name: contract_templates_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.contract_templates_id_seq', 7, true);


--
-- TOC entry 5250 (class 0 OID 0)
-- Dependencies: 228
-- Name: insurance_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.insurance_requests_id_seq', 36, true);


--
-- TOC entry 5251 (class 0 OID 0)
-- Dependencies: 230
-- Name: loan_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.loan_requests_id_seq', 26, true);


--
-- TOC entry 5252 (class 0 OID 0)
-- Dependencies: 232
-- Name: order_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.order_requests_id_seq', 11, true);


--
-- TOC entry 5253 (class 0 OID 0)
-- Dependencies: 234
-- Name: purchase_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.purchase_requests_id_seq', 10, true);


--
-- TOC entry 5254 (class 0 OID 0)
-- Dependencies: 236
-- Name: purchases_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.purchases_id_seq', 44, true);


--
-- TOC entry 5255 (class 0 OID 0)
-- Dependencies: 238
-- Name: rental_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.rental_requests_id_seq', 35, true);


--
-- TOC entry 5256 (class 0 OID 0)
-- Dependencies: 240
-- Name: service_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.service_requests_id_seq', 21, true);


--
-- TOC entry 5257 (class 0 OID 0)
-- Dependencies: 242
-- Name: test_drives_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.test_drives_id_seq', 15, true);


--
-- TOC entry 4978 (class 2606 OID 16558)
-- Name: admins admins_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_pkey PRIMARY KEY (id);


--
-- TOC entry 4980 (class 2606 OID 16560)
-- Name: admins admins_username_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_username_key UNIQUE (username);


--
-- TOC entry 4982 (class 2606 OID 16562)
-- Name: car_types car_types_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types
    ADD CONSTRAINT car_types_pkey PRIMARY KEY (id);


--
-- TOC entry 4984 (class 2606 OID 16564)
-- Name: cars cars_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT cars_pkey PRIMARY KEY (id);


--
-- TOC entry 4986 (class 2606 OID 16566)
-- Name: clients clients_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_email_key UNIQUE (email);


--
-- TOC entry 4988 (class 2606 OID 16568)
-- Name: clients clients_phone_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_phone_key UNIQUE (phone);


--
-- TOC entry 4990 (class 2606 OID 16570)
-- Name: clients clients_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_pkey PRIMARY KEY (id);


--
-- TOC entry 5008 (class 2606 OID 115200)
-- Name: contract_templates contract_templates_code_version_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.contract_templates
    ADD CONSTRAINT contract_templates_code_version_key UNIQUE (code, version);


--
-- TOC entry 5010 (class 2606 OID 115198)
-- Name: contract_templates contract_templates_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.contract_templates
    ADD CONSTRAINT contract_templates_pkey PRIMARY KEY (id);


--
-- TOC entry 4992 (class 2606 OID 16572)
-- Name: insurance_requests insurance_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4994 (class 2606 OID 16574)
-- Name: loan_requests loan_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4996 (class 2606 OID 16576)
-- Name: order_requests order_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.order_requests
    ADD CONSTRAINT order_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4998 (class 2606 OID 16578)
-- Name: purchase_requests purchase_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchase_requests
    ADD CONSTRAINT purchase_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 5000 (class 2606 OID 16580)
-- Name: purchases purchases_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_pkey PRIMARY KEY (id);


--
-- TOC entry 5002 (class 2606 OID 16582)
-- Name: rental_requests rental_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rental_requests
    ADD CONSTRAINT rental_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 5004 (class 2606 OID 16584)
-- Name: service_requests service_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 5006 (class 2606 OID 16586)
-- Name: test_drives test_drives_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_pkey PRIMARY KEY (id);


--
-- TOC entry 5027 (class 2620 OID 16587)
-- Name: insurance_requests handle_insurance_approval; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER handle_insurance_approval AFTER UPDATE ON public.insurance_requests FOR EACH ROW EXECUTE FUNCTION public.handle_approved_insurance_request();


--
-- TOC entry 5030 (class 2620 OID 25126)
-- Name: loan_requests handle_loan_approval; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER handle_loan_approval AFTER UPDATE ON public.loan_requests FOR EACH ROW EXECUTE FUNCTION public.handle_approved_loan_request();


--
-- TOC entry 5033 (class 2620 OID 115180)
-- Name: order_requests handle_order_approval; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER handle_order_approval AFTER UPDATE ON public.order_requests FOR EACH ROW EXECUTE FUNCTION public.handle_approved_order_request();


--
-- TOC entry 5036 (class 2620 OID 115179)
-- Name: purchase_requests handle_purchase_approval; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER handle_purchase_approval AFTER UPDATE ON public.purchase_requests FOR EACH ROW EXECUTE FUNCTION public.handle_approved_purchase_request();


--
-- TOC entry 5028 (class 2620 OID 16591)
-- Name: insurance_requests mark_insurance_viewed; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER mark_insurance_viewed BEFORE UPDATE ON public.insurance_requests FOR EACH ROW WHEN (((old.notification_shown = false) AND (new.notification_shown = true))) EXECUTE FUNCTION public.mark_notification_as_viewed();


--
-- TOC entry 5031 (class 2620 OID 16592)
-- Name: loan_requests mark_loan_viewed; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER mark_loan_viewed BEFORE UPDATE ON public.loan_requests FOR EACH ROW WHEN (((old.notification_shown = false) AND (new.notification_shown = true))) EXECUTE FUNCTION public.mark_notification_as_viewed();


--
-- TOC entry 5034 (class 2620 OID 16593)
-- Name: order_requests mark_order_viewed; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER mark_order_viewed BEFORE UPDATE ON public.order_requests FOR EACH ROW WHEN (((old.notification_shown = false) AND (new.notification_shown = true))) EXECUTE FUNCTION public.mark_notification_as_viewed();


--
-- TOC entry 5037 (class 2620 OID 16594)
-- Name: purchase_requests mark_purchase_viewed; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER mark_purchase_viewed BEFORE UPDATE ON public.purchase_requests FOR EACH ROW WHEN (((old.notification_shown = false) AND (new.notification_shown = true))) EXECUTE FUNCTION public.mark_notification_as_viewed();


--
-- TOC entry 5040 (class 2620 OID 16595)
-- Name: rental_requests mark_rental_viewed; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER mark_rental_viewed BEFORE UPDATE ON public.rental_requests FOR EACH ROW WHEN (((old.notification_shown = false) AND (new.notification_shown = true))) EXECUTE FUNCTION public.mark_notification_as_viewed();


--
-- TOC entry 5043 (class 2620 OID 16596)
-- Name: service_requests mark_service_viewed; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER mark_service_viewed BEFORE UPDATE ON public.service_requests FOR EACH ROW WHEN (((old.notification_shown = false) AND (new.notification_shown = true))) EXECUTE FUNCTION public.mark_notification_as_viewed();


--
-- TOC entry 5045 (class 2620 OID 16597)
-- Name: test_drives mark_test_drive_viewed; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER mark_test_drive_viewed BEFORE UPDATE ON public.test_drives FOR EACH ROW WHEN (((old.notification_shown = false) AND (new.notification_shown = true))) EXECUTE FUNCTION public.mark_notification_as_viewed();


--
-- TOC entry 5029 (class 2620 OID 16598)
-- Name: insurance_requests reset_insurance_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER reset_insurance_notification BEFORE UPDATE ON public.insurance_requests FOR EACH ROW EXECUTE FUNCTION public.reset_notification_on_status_change();


--
-- TOC entry 5032 (class 2620 OID 16599)
-- Name: loan_requests reset_loan_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER reset_loan_notification BEFORE UPDATE ON public.loan_requests FOR EACH ROW EXECUTE FUNCTION public.reset_notification_on_status_change();


--
-- TOC entry 5035 (class 2620 OID 16600)
-- Name: order_requests reset_order_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER reset_order_notification BEFORE UPDATE ON public.order_requests FOR EACH ROW EXECUTE FUNCTION public.reset_notification_on_status_change();


--
-- TOC entry 5038 (class 2620 OID 16601)
-- Name: purchase_requests reset_purchase_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER reset_purchase_notification BEFORE UPDATE ON public.purchase_requests FOR EACH ROW EXECUTE FUNCTION public.reset_notification_on_status_change();


--
-- TOC entry 5041 (class 2620 OID 16602)
-- Name: rental_requests reset_rental_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER reset_rental_notification BEFORE UPDATE ON public.rental_requests FOR EACH ROW EXECUTE FUNCTION public.reset_notification_on_status_change();


--
-- TOC entry 5044 (class 2620 OID 16603)
-- Name: service_requests reset_service_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER reset_service_notification BEFORE UPDATE ON public.service_requests FOR EACH ROW EXECUTE FUNCTION public.reset_notification_on_status_change();


--
-- TOC entry 5046 (class 2620 OID 16604)
-- Name: test_drives reset_test_drive_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER reset_test_drive_notification BEFORE UPDATE ON public.test_drives FOR EACH ROW EXECUTE FUNCTION public.reset_notification_on_status_change();


--
-- TOC entry 5039 (class 2620 OID 16605)
-- Name: purchase_requests validate_purchase_availability_trigger; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER validate_purchase_availability_trigger BEFORE INSERT ON public.purchase_requests FOR EACH ROW EXECUTE FUNCTION public.validate_purchase_availability();


--
-- TOC entry 5042 (class 2620 OID 16606)
-- Name: rental_requests validate_rental_availability_trigger; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER validate_rental_availability_trigger BEFORE INSERT ON public.rental_requests FOR EACH ROW EXECUTE FUNCTION public.validate_rental_availability();


--
-- TOC entry 5047 (class 2620 OID 16607)
-- Name: test_drives validate_test_drive_availability_trigger; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER validate_test_drive_availability_trigger BEFORE INSERT ON public.test_drives FOR EACH ROW EXECUTE FUNCTION public.validate_test_drive_availability();


--
-- TOC entry 5011 (class 2606 OID 16608)
-- Name: cars fk_type; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT fk_type FOREIGN KEY (type_id) REFERENCES public.car_types(id) ON DELETE CASCADE;


--
-- TOC entry 5012 (class 2606 OID 16613)
-- Name: insurance_requests insurance_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 5013 (class 2606 OID 16618)
-- Name: insurance_requests insurance_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 5014 (class 2606 OID 16623)
-- Name: loan_requests loan_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 5015 (class 2606 OID 16628)
-- Name: loan_requests loan_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 5016 (class 2606 OID 16633)
-- Name: order_requests order_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.order_requests
    ADD CONSTRAINT order_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 5017 (class 2606 OID 16638)
-- Name: purchase_requests purchase_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchase_requests
    ADD CONSTRAINT purchase_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 5018 (class 2606 OID 16643)
-- Name: purchase_requests purchase_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchase_requests
    ADD CONSTRAINT purchase_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 5019 (class 2606 OID 16648)
-- Name: purchases purchases_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 5020 (class 2606 OID 16653)
-- Name: purchases purchases_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 5021 (class 2606 OID 16658)
-- Name: rental_requests rental_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rental_requests
    ADD CONSTRAINT rental_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 5022 (class 2606 OID 16663)
-- Name: rental_requests rental_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rental_requests
    ADD CONSTRAINT rental_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 5023 (class 2606 OID 16668)
-- Name: service_requests service_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 5024 (class 2606 OID 16673)
-- Name: service_requests service_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 5025 (class 2606 OID 16678)
-- Name: test_drives test_drives_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 5026 (class 2606 OID 16683)
-- Name: test_drives test_drives_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


-- Completed on 2026-06-22 21:28:45

--
-- PostgreSQL database dump complete
--

\unrestrict xexo4BGoEvzPJg1Esljmp3rfvxzHMjs13rrqjnprKWvpqCwHOcfFEHS1NhukuFi

