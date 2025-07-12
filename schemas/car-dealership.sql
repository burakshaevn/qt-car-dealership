--
-- PostgreSQL database dump
--

-- Dumped from database version 17.4
-- Dumped by pg_dump version 17.4

-- Started on 2025-07-12 09:48:37

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
-- TOC entry 239 (class 1255 OID 49313)
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
-- TOC entry 238 (class 1255 OID 49311)
-- Name: handle_approved_loan_request(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.handle_approved_loan_request() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF NEW.status = 'approved' AND OLD.status != 'approved' THEN
        INSERT INTO purchases (car_id, client_id, тип_оплаты, сумма_кредита, срок_кредита_месяцев)
        VALUES (NEW.car_id, NEW.client_id, 'кредит', NEW.loan_amount, NEW.loan_term_months);
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.handle_approved_loan_request() OWNER TO postgres;

--
-- TOC entry 237 (class 1255 OID 49307)
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

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- TOC entry 217 (class 1259 OID 16469)
-- Name: admins; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.admins (
    id integer NOT NULL,
    username character varying(100) NOT NULL,
    password text NOT NULL
);


ALTER TABLE public.admins OWNER TO postgres;

--
-- TOC entry 5044 (class 0 OID 0)
-- Dependencies: 217
-- Name: TABLE admins; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.admins IS 'Администраторы системы';


--
-- TOC entry 218 (class 1259 OID 16474)
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
-- TOC entry 5045 (class 0 OID 0)
-- Dependencies: 218
-- Name: admins_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.admins_id_seq OWNED BY public.admins.id;


--
-- TOC entry 219 (class 1259 OID 16475)
-- Name: car_types; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.car_types (
    id integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.car_types OWNER TO postgres;

--
-- TOC entry 5046 (class 0 OID 0)
-- Dependencies: 219
-- Name: TABLE car_types; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.car_types IS 'Типы автомобилей';


--
-- TOC entry 220 (class 1259 OID 16480)
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
-- TOC entry 5047 (class 0 OID 0)
-- Dependencies: 220
-- Name: car_types_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.car_types_id_seq OWNED BY public.car_types.id;


--
-- TOC entry 221 (class 1259 OID 16481)
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
    available_for_rent boolean DEFAULT true NOT NULL
);


ALTER TABLE public.cars OWNER TO postgres;

--
-- TOC entry 5048 (class 0 OID 0)
-- Dependencies: 221
-- Name: TABLE cars; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.cars IS 'Автомобили в наличии';


--
-- TOC entry 222 (class 1259 OID 16486)
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
-- TOC entry 5049 (class 0 OID 0)
-- Dependencies: 222
-- Name: cars_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.cars_id_seq OWNED BY public.cars.id;


--
-- TOC entry 223 (class 1259 OID 16487)
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
-- TOC entry 5050 (class 0 OID 0)
-- Dependencies: 223
-- Name: TABLE clients; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.clients IS 'Клиенты автосалона';


--
-- TOC entry 224 (class 1259 OID 16492)
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
-- TOC entry 5051 (class 0 OID 0)
-- Dependencies: 224
-- Name: clients_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.clients_id_seq OWNED BY public.clients.id;


--
-- TOC entry 231 (class 1259 OID 16592)
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
    CONSTRAINT insurance_requests_insurance_type_check CHECK (((insurance_type)::text = ANY ((ARRAY['ОСАГО'::character varying, 'КАСКО'::character varying, 'Комплекс'::character varying])::text[]))),
    CONSTRAINT insurance_requests_status_check CHECK (((status)::text = ANY ((ARRAY['не обработано'::character varying, 'одобрено'::character varying, 'отклонено'::character varying, 'завершено'::character varying])::text[])))
);


ALTER TABLE public.insurance_requests OWNER TO postgres;

--
-- TOC entry 5052 (class 0 OID 0)
-- Dependencies: 231
-- Name: TABLE insurance_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.insurance_requests IS 'Заявки на страхование';


--
-- TOC entry 232 (class 1259 OID 16611)
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
-- TOC entry 5053 (class 0 OID 0)
-- Dependencies: 232
-- Name: insurance_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.insurance_requests_id_seq OWNED BY public.insurance_requests.id;


--
-- TOC entry 229 (class 1259 OID 16570)
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
-- TOC entry 5054 (class 0 OID 0)
-- Dependencies: 229
-- Name: TABLE loan_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.loan_requests IS 'Заявки на кредитование';


--
-- TOC entry 230 (class 1259 OID 16590)
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
-- TOC entry 5055 (class 0 OID 0)
-- Dependencies: 230
-- Name: loan_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.loan_requests_id_seq OWNED BY public.loan_requests.id;


--
-- TOC entry 225 (class 1259 OID 16493)
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
    CONSTRAINT "проверка_типа_оплаты" CHECK ((("тип_оплаты")::text = ANY ((ARRAY['наличные'::character varying, 'кредит'::character varying])::text[]))),
    CONSTRAINT "проверка_типа_страховки" CHECK ((("тип_страховки")::text = ANY ((ARRAY['ОСАГО'::character varying, 'КАСКО'::character varying, 'Комплекс'::character varying])::text[])))
);


ALTER TABLE public.purchases OWNER TO postgres;

--
-- TOC entry 5056 (class 0 OID 0)
-- Dependencies: 225
-- Name: TABLE purchases; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.purchases IS 'История покупок автомобилей';


--
-- TOC entry 226 (class 1259 OID 16497)
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
-- TOC entry 5057 (class 0 OID 0)
-- Dependencies: 226
-- Name: purchases_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.purchases_id_seq OWNED BY public.purchases.id;


--
-- TOC entry 236 (class 1259 OID 49320)
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
-- TOC entry 5058 (class 0 OID 0)
-- Dependencies: 236
-- Name: TABLE rental_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.rental_requests IS 'Заявки на аренду автомобилей';


--
-- TOC entry 235 (class 1259 OID 49319)
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
-- TOC entry 233 (class 1259 OID 16613)
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
    CONSTRAINT service_requests_status_check CHECK (((status)::text = ANY ((ARRAY['не обработано'::character varying, 'подтверждено'::character varying, 'выполнено'::character varying, 'отменено'::character varying])::text[])))
);


ALTER TABLE public.service_requests OWNER TO postgres;

--
-- TOC entry 5059 (class 0 OID 0)
-- Dependencies: 233
-- Name: TABLE service_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.service_requests IS 'Заявки на сервисное обслуживание';


--
-- TOC entry 234 (class 1259 OID 16631)
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
-- TOC entry 5060 (class 0 OID 0)
-- Dependencies: 234
-- Name: service_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.service_requests_id_seq OWNED BY public.service_requests.id;


--
-- TOC entry 227 (class 1259 OID 16534)
-- Name: test_drives; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.test_drives (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    scheduled_date timestamp without time zone NOT NULL,
    status character varying(20) DEFAULT 'pending'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT test_drives_status_check CHECK (((status)::text = ANY (ARRAY[('не обработано'::character varying)::text, ('подтверждено'::character varying)::text, ('выполнено'::character varying)::text, ('отменено'::character varying)::text])))
);


ALTER TABLE public.test_drives OWNER TO postgres;

--
-- TOC entry 5061 (class 0 OID 0)
-- Dependencies: 227
-- Name: TABLE test_drives; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.test_drives IS 'Заявки на тест-драйв';


--
-- TOC entry 228 (class 1259 OID 16552)
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
-- TOC entry 5062 (class 0 OID 0)
-- Dependencies: 228
-- Name: test_drives_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.test_drives_id_seq OWNED BY public.test_drives.id;


--
-- TOC entry 4790 (class 2604 OID 41110)
-- Name: admins id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins ALTER COLUMN id SET DEFAULT nextval('public.admins_id_seq'::regclass);


--
-- TOC entry 4791 (class 2604 OID 41111)
-- Name: car_types id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types ALTER COLUMN id SET DEFAULT nextval('public.car_types_id_seq'::regclass);


--
-- TOC entry 4792 (class 2604 OID 41112)
-- Name: cars id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars ALTER COLUMN id SET DEFAULT nextval('public.cars_id_seq'::regclass);


--
-- TOC entry 4794 (class 2604 OID 41113)
-- Name: clients id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients ALTER COLUMN id SET DEFAULT nextval('public.clients_id_seq'::regclass);


--
-- TOC entry 4805 (class 2604 OID 41114)
-- Name: insurance_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests ALTER COLUMN id SET DEFAULT nextval('public.insurance_requests_id_seq'::regclass);


--
-- TOC entry 4801 (class 2604 OID 41115)
-- Name: loan_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests ALTER COLUMN id SET DEFAULT nextval('public.loan_requests_id_seq'::regclass);


--
-- TOC entry 4795 (class 2604 OID 41116)
-- Name: purchases id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases ALTER COLUMN id SET DEFAULT nextval('public.purchases_id_seq'::regclass);


--
-- TOC entry 4809 (class 2604 OID 41117)
-- Name: service_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests ALTER COLUMN id SET DEFAULT nextval('public.service_requests_id_seq'::regclass);


--
-- TOC entry 4798 (class 2604 OID 41118)
-- Name: test_drives id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives ALTER COLUMN id SET DEFAULT nextval('public.test_drives_id_seq'::regclass);


--
-- TOC entry 5019 (class 0 OID 16469)
-- Dependencies: 217
-- Data for Name: admins; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.admins VALUES (1, 'admin1', 'adminpass1');
INSERT INTO public.admins VALUES (2, 'admin2', 'adminpass2');
INSERT INTO public.admins VALUES (3, 'admin3', 'adminpass3');


--
-- TOC entry 5021 (class 0 OID 16475)
-- Dependencies: 219
-- Data for Name: car_types; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.car_types VALUES (2, 'Внедорожник');
INSERT INTO public.car_types VALUES (3, 'Купе');
INSERT INTO public.car_types VALUES (4, 'Кабриолет');
INSERT INTO public.car_types VALUES (1, 'Лимузин');


--
-- TOC entry 5023 (class 0 OID 16481)
-- Dependencies: 221
-- Data for Name: cars; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.cars VALUES (1, 'CLE 200 Cabriolet', 'Чёрный', 5683000, 'Элегантный кабриолет с современным дизайном. Идеальный выбор для комфортных поездок по городу.', 'CLE 200 Cabriolet\black.png', 4, true);
INSERT INTO public.cars VALUES (3, 'CLE 200 Cabriolet', 'Красный', 5683000, 'Яркий красный кабриолет с спортивным характером. Вместительный салон и премиальная отделка.', 'CLE 200 Cabriolet\red.png', 4, true);
INSERT INTO public.cars VALUES (4, 'CLE 200 Cabriolet', 'Белый', 5683000, 'Белоснежный кабриолет с роскошным интерьером. Современные технологии и высокий уровень комфорта.', 'CLE 200 Cabriolet\white.png', 4, true);
INSERT INTO public.cars VALUES (5, 'S 63 E Performance', 'Золотой магнолитет', 20520000, 'Роскошный седан в эксклюзивном золотом цвете. Мощный гибридный двигатель и инновационные технологии.', 'Mercedes-AMG S 63 E Performance\kalaharigold magno.png', 1, true);
INSERT INTO public.cars VALUES (2, 'CLE 200 Cabriolet', 'Синий', 5683000, 'Стильный кабриолет в голубом цвете. Отличная динамика и топливная экономичность.', 'CLE 200 Cabriolet\blue.png', 4, true);
INSERT INTO public.cars VALUES (6, 'S 63 E Performance', 'Синий', 20520000, 'Изысканный седан в мистическом синем цвете. Превосходная управляемость и максимальный комфорт.', 'Mercedes-AMG S 63 E Performance\mysticblau metallic.png', 1, true);
INSERT INTO public.cars VALUES (9, 'S 63 E Performance', 'Белый', 20520000, 'Белоснежный представительский седан. Просторный салон с роскошной отделкой и передовыми системами.', 'Mercedes-AMG S 63 E Performance\opalithwei bright.png', 1, true);
INSERT INTO public.cars VALUES (12, 'EQS 53 4MATIC+', 'Белый', 15700000, 'Футуристичный электромобиль в белом цвете. Инновационный дизайн и выдающаяся дальность хода.', 'Mercedes-AMG EQS 53 4MATIC+\white.png', 1, true);
INSERT INTO public.cars VALUES (13, 'EQS 53 4MATIC+', 'Красный', 15700000, 'Динамичный электрокар в красном цвете. Мгновенный разгон и премиальное оснащение.', 'Mercedes-AMG EQS 53 4MATIC+\red.png', 1, true);
INSERT INTO public.cars VALUES (14, 'EQS 53 4MATIC+', 'Чёрный', 15700000, 'Элегантный чёрный электромобиль. Тихая и плавная езда с высоким уровнем комфорта.', 'Mercedes-AMG EQS 53 4MATIC+\black.png', 1, true);
INSERT INTO public.cars VALUES (15, 'Mercedes-AMG G 63', 'Бриллиант', 19890000, 'Легендарный внедорожник в бриллиантово-синем цвете. Непревзойдённая проходимость и мощный двигатель.', 'Mercedes-AMG G 63\blue.png', 2, true);
INSERT INTO public.cars VALUES (16, 'Mercedes-AMG G 63', 'Белый', 19890000, 'Классический белый внедорожник. Просторный салон и передовые системы безопасности.', 'Mercedes-AMG G 63\white.png', 2, true);
INSERT INTO public.cars VALUES (17, 'Mercedes-AMG G 63', 'Серый', 19890000, 'Серый внедорожник с брутальным дизайном. Отличная управляемость на любом покрытии.', 'Mercedes-AMG G 63\gray.png', 2, true);
INSERT INTO public.cars VALUES (18, 'Mercedes-AMG G 63', 'Зелёный', 19890000, 'Зелёный внедорожник для ценителей стиля. Мощный двигатель и комфортабельный интерьер.', 'Mercedes-AMG G 63\green.png', 2, true);
INSERT INTO public.cars VALUES (19, 'Mercedes-AMG G 63', 'Красный', 19890000, 'Яркий красный внедорожник с агрессивным дизайном. Впечатляющая динамика и проходимость.', 'Mercedes-AMG G 63\red.png', 2, true);
INSERT INTO public.cars VALUES (20, 'Mercedes-AMG G 63', 'Жёлтый', 19890000, 'Жёлтый внедорожник для смелых водителей. Уникальный внешний вид и выдающиеся характеристики.', 'Mercedes-AMG G 63\yellow.png', 2, true);
INSERT INTO public.cars VALUES (21, 'Mercedes-AMG GLB 35 4MATIC', 'Чёрный', 6840000, 'Компактный кроссовер в чёрном цвете. Универсальный автомобиль для города и путешествий.', 'Mercedes-AMG GLB 35 4MATIC\black.png', 2, true);
INSERT INTO public.cars VALUES (22, 'Mercedes-AMG GLB 35 4MATIC', 'Синий', 6840000, 'Стильный синий кроссовер с динамичным характером. Вместительный салон и экономичный двигатель.', 'Mercedes-AMG GLB 35 4MATIC\blue.png', 2, true);
INSERT INTO public.cars VALUES (23, 'Mercedes-AMG GLB 35 4MATIC', 'Красный', 6840000, 'Энергичный красный кроссовер. Отличная управляемость и современные технологии.', 'Mercedes-AMG GLB 35 4MATIC\red.png', 2, true);
INSERT INTO public.cars VALUES (24, 'Mercedes-AMG GLB 35 4MATIC', 'Белый', 6840000, 'Белый кроссовер с элегантным дизайном. Комфорт и практичность в одном автомобиле.', 'Mercedes-AMG GLB 35 4MATIC\white.png', 2, true);
INSERT INTO public.cars VALUES (25, 'GLC 43 4MATIC Coupé', 'Чёрный', 8840000, 'Спортивный купе-кроссовер в чёрном цвете. Динамичный дизайн и мощный двигатель.', 'Mercedes-AMG GLC 43 4MATIC Coupé\black.png', 3, true);
INSERT INTO public.cars VALUES (27, 'GLC 43 4MATIC Coupé', 'Красный', 8840000, 'Яркий красный купе-кроссовер. Спортивный характер и премиальное оснащение.', 'Mercedes-AMG GLC 43 4MATIC Coupé\red.png', 3, true);
INSERT INTO public.cars VALUES (28, 'GLC 43 4MATIC Coupé', 'Белый', 8840000, 'Белоснежный купе-кроссовер с роскошным интерьером. Комфорт и динамика в одном автомобиле.', 'Mercedes-AMG GLC 43 4MATIC Coupé\white.png', 3, true);
INSERT INTO public.cars VALUES (38, 'GLS 63 4MATIC+', 'Чёрный', 18495000, 'Большой чёрный внедорожник премиум-класса. Просторный салон и мощный двигатель.', 'Mercedes-AMG GLS 63 4MATIC+\black.png', 2, true);
INSERT INTO public.cars VALUES (40, 'GLS 63 4MATIC+', 'Зелёный', 18495000, 'Зелёный внедорожник для любителей природы. Вместительный и технологичный автомобиль.', 'Mercedes-AMG GLS 63 4MATIC+\green.png', 2, true);
INSERT INTO public.cars VALUES (41, 'GLS 63 4MATIC+', 'Красный', 18495000, 'Красный внедорожник с ярким характером. Мощность и роскошь в одном автомобиле.', 'Mercedes-AMG GLS 63 4MATIC+\red.png', 2, true);
INSERT INTO public.cars VALUES (42, 'GLS 63 4MATIC+', 'Серый', 18495000, 'Серый внедорожник с элегантным дизайном. Идеальный выбор для семьи и путешествий.', 'Mercedes-AMG GLS 63 4MATIC+\gray.png', 2, true);
INSERT INTO public.cars VALUES (68, 'GLS 63 4MATIC+', 'Белый', 18495000, 'Белый внедорожник премиум-класса. Просторный салон и передовые технологии.', 'Mercedes-AMG GLS 63 4MATIC+\white.png', 2, true);
INSERT INTO public.cars VALUES (43, 'Mercedes-AMG SL 43', 'Чёрный', 5563000, 'Роскошный чёрный родстер. Идеальный автомобиль для любителей открытых дорог.', 'Mercedes-AMG SL 43\black.png', 4, true);
INSERT INTO public.cars VALUES (44, 'Mercedes-AMG SL 43', 'Голубой', 5563000, 'Элегантный голубой родстер. Сочетание стиля и выдающихся динамических характеристик.', 'Mercedes-AMG SL 43\blue.png', 4, true);
INSERT INTO public.cars VALUES (45, 'Mercedes-AMG SL 43', 'Красный', 5563000, 'Страстный красный родстер. Спортивный характер и неповторимый дизайн.', 'Mercedes-AMG SL 43\red.png', 4, true);
INSERT INTO public.cars VALUES (7, 'S 63 E Performance', 'Чёрный', 20520000, 'Стильный чёрный седан с матовым покрытием. Впечатляющая мощность и динамичные характеристики.', 'Mercedes-AMG S 63 E Performance\nachtschwarz magno.png', 1, true);
INSERT INTO public.cars VALUES (10, 'S 63 E Performance', 'Голубой', 20520000, 'Эксклюзивный седан в винтажно-голубом цвете. Сочетание классического стиля и современных технологий.', 'Mercedes-AMG S 63 E Performance\vintageblau uni.png', 1, true);
INSERT INTO public.cars VALUES (39, 'GLS 63 4MATIC+', 'Синий', 18495000, 'Голубой внедорожник с представительским характером. Высокий уровень комфорта и безопасности.', 'Mercedes-AMG GLS 63 4MATIC+\blue.png', 2, true);
INSERT INTO public.cars VALUES (46, 'Mercedes-AMG SL 43', 'Белый', 5563000, 'Белоснежный родстер с премиальным интерьером. Комфорт и удовольствие от вождения.', 'Mercedes-AMG SL 43\white.png', 4, true);
INSERT INTO public.cars VALUES (47, 'Mercedes-AMG SL 43', 'Жёлтый', 5563000, 'Яркий жёлтый родстер для смелых водителей. Динамичный дизайн и мощный двигатель.', 'Mercedes-AMG SL 43\yellow.png', 4, true);
INSERT INTO public.cars VALUES (48, 'Mercedes-AMG GT 43', 'Чёрный', 12209000, 'Спортивный чёрный седан. Мощный двигатель и агрессивный дизайн.', 'Mercedes-AMG GT 43\black.png', 3, true);
INSERT INTO public.cars VALUES (49, 'Mercedes-AMG GT 43', 'Серый', 12209000, 'Элегантный серый седан с динамичным характером. Премиальное оснащение и комфорт.', 'Mercedes-AMG GT 43\gray.png', 3, true);
INSERT INTO public.cars VALUES (50, 'Mercedes-AMG GT 43', 'Зелёный', 12209000, 'Эксклюзивный зелёный седан. Уникальный внешний вид и выдающиеся характеристики.', 'Mercedes-AMG GT 43\green.png', 3, true);
INSERT INTO public.cars VALUES (51, 'Mercedes-AMG GT 43', 'Голубой', 12209000, 'Яркий голубой седан с футуристичным дизайном. Современные технологии и динамика.', 'Mercedes-AMG GT 43\hyperblau.png', 3, true);
INSERT INTO public.cars VALUES (52, 'Mercedes-AMG GT 43', 'Красный', 12209000, 'Страстный красный седан. Спортивный характер и роскошный интерьер.', 'Mercedes-AMG GT 43\red.png', 3, true);
INSERT INTO public.cars VALUES (53, 'Mercedes-AMG GT 43', 'Белый', 12209000, 'Белоснежный седан премиум-класса. Сочетание элегантности и мощности.', 'Mercedes-AMG GT 43\white.png', 3, true);
INSERT INTO public.cars VALUES (59, 'GT 63 S 4MATIC+', 'Чёрный', 8420000, 'Агрессивный чёрный спортивный автомобиль. Мощный двигатель и выдающаяся динамика.', 'Mercedes-AMG GT 63 S 4MATIC+\black.png', 3, true);
INSERT INTO public.cars VALUES (60, 'GT 63 S 4MATIC+', 'Зелёный', 8420000, 'Эксклюзивный зелёный спортивный автомобиль. Уникальный дизайн и премиальное оснащение.', 'Mercedes-AMG GT 63 S 4MATIC+\green.png', 3, true);
INSERT INTO public.cars VALUES (61, 'GT 63 S 4MATIC+', 'Красный', 8420000, 'Страстный красный спортивный автомобиль. Идеальный выбор для любителей скорости.', 'Mercedes-AMG GT 63 S 4MATIC+\red.png', 3, true);
INSERT INTO public.cars VALUES (62, 'GT 63 S 4MATIC+', 'Белый', 8420000, 'Элегантный белый спортивный автомобиль. Сочетание стиля и мощности.', 'Mercedes-AMG GT 63 S 4MATIC+\white.png', 3, true);
INSERT INTO public.cars VALUES (63, 'GT 63 S 4MATIC+', 'Жёлтый', 8420000, 'Яркий жёлтый спортивный автомобиль. Неповторимый внешний вид и динамичные характеристики.', 'Mercedes-AMG GT 63 S 4MATIC+\yellow.png', 3, true);
INSERT INTO public.cars VALUES (64, 'CLE 53 4MATIC+ Coupé', 'Чёрный', 9051000, 'Стильный чёрный купе. Динамичный дизайн и мощный двигатель.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\black.png', 3, true);
INSERT INTO public.cars VALUES (65, 'CLE 53 4MATIC+ Coupé', 'Красный', 9051000, 'Энергичный красный купе. Спортивный характер и современные технологии.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\red.png', 3, true);
INSERT INTO public.cars VALUES (66, 'CLE 53 4MATIC+ Coupé', 'Белый', 9051000, 'Элегантный белый купе. Роскошный интерьер и комфортабельный салон.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\white.png', 3, true);
INSERT INTO public.cars VALUES (67, 'CLE 53 4MATIC+ Coupé', 'Жёлтый', 9051000, 'Яркий жёлтый купе для смелых водителей. Уникальный внешний вид и выдающаяся динамика.', 'Mercedes-AMG CLE 53 4MATIC+ Coupé\yellow.png', 3, true);
INSERT INTO public.cars VALUES (26, 'GLC 43 4MATIC Coupé', 'Синий', 8840000, 'Элегантный голубой купе-кроссовер. Сочетание стиля и выдающихся ходовых качеств.', 'Mercedes-AMG GLC 43 4MATIC Coupé\blue.png', 3, true);
INSERT INTO public.cars VALUES (69, 'E 220 d Limousine', 'Белый', 6077570, NULL, 'E 220 d Limousine\white.png', 1, true);
INSERT INTO public.cars VALUES (70, 'E 220 d Limousine', 'Чёрный', 6077570, NULL, 'E 220 d Limousine\black.png', 1, true);
INSERT INTO public.cars VALUES (71, 'E 220 d Limousine', 'Красный', 6077570, NULL, 'E 220 d Limousine\red.png', 1, true);
INSERT INTO public.cars VALUES (72, 'Mercedes-Maybach SL 680 Monogram Series', 'Красный', 24219570, NULL, 'Mercedes-Maybach SL 680 Monogram Series\red.png', 4, true);
INSERT INTO public.cars VALUES (73, 'Mercedes-Maybach SL 680 Monogram Series', 'Белый', 24219570, NULL, 'Mercedes-Maybach SL 680 Monogram Series\white.png', 4, true);
INSERT INTO public.cars VALUES (74, 'Mercedes-Maybach SL 680 Monogram Series', 'Чёрный', 24219570, NULL, 'Mercedes-Maybach SL 680 Monogram Series\black.png', 4, true);
INSERT INTO public.cars VALUES (75, 'Mercedes-Maybach SL 680 Monogram Series', 'Жёлтый', 24219570, NULL, 'Mercedes-Maybach SL 680 Monogram Series\yellow.png', 4, true);
INSERT INTO public.cars VALUES (76, 'Mercedes-Maybach SL 680 Monogram Series', 'Синий', 24219570, NULL, 'Mercedes-Maybach SL 680 Monogram Series\blue.png', 4, true);


--
-- TOC entry 5025 (class 0 OID 16487)
-- Dependencies: 223
-- Data for Name: clients; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.clients VALUES (7, 'Никита', 'Буракшаев', '79274800234', 'nb@example.com', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f');


--
-- TOC entry 5033 (class 0 OID 16592)
-- Dependencies: 231
-- Data for Name: insurance_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.insurance_requests VALUES (14, 7, 21, 'Комплекс', 'одобрено', '2025-06-23 09:47:39.108954', true);


--
-- TOC entry 5031 (class 0 OID 16570)
-- Dependencies: 229
-- Data for Name: loan_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.loan_requests VALUES (5, 7, 45, 5563000, 12, 'отклонено', '2025-06-05 09:29:08.70719', true);
INSERT INTO public.loan_requests VALUES (6, 7, 1, 100004, 2, 'одобрено', '2025-06-23 09:41:40.572766', true);


--
-- TOC entry 5027 (class 0 OID 16493)
-- Dependencies: 225
-- Data for Name: purchases; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.purchases VALUES (22, 60, 7, '2025-06-04 15:43:24.990196', 'наличные', NULL, NULL, NULL);
INSERT INTO public.purchases VALUES (23, 21, 7, '2025-06-23 09:48:01.749137', 'наличные', NULL, NULL, 'Комплекс');


--
-- TOC entry 5038 (class 0 OID 49320)
-- Dependencies: 236
-- Data for Name: rental_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (4, 7, 4, 1, '2025-06-06', 'отклонено', '2025-06-06 08:23:09.181534', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (5, 7, 43, 1, '2025-06-06', 'отклонено', '2025-06-06 08:32:01.146866', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (3, 7, 2, 1, '2025-06-06', 'отклонено', '2025-06-06 08:21:09.485995', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (2, 7, 76, 1, '2025-06-06', 'отклонено', '2025-06-06 08:05:12.155505', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (7, 7, 4, 1, '2025-06-23', 'одобрено', '2025-06-23 09:35:19.63823', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (1, 7, 2, 7, '2025-06-06', 'отклонено', '2025-06-06 06:42:12.853094', true);
INSERT INTO public.rental_requests OVERRIDING SYSTEM VALUE VALUES (6, 7, 3, 4, '2025-06-23', 'отклонено', '2025-06-23 09:23:04.814957', true);


--
-- TOC entry 5035 (class 0 OID 16613)
-- Dependencies: 233
-- Data for Name: service_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.service_requests VALUES (7, 7, 1, 'Замена масла', '2025-06-27 09:00:00', 'подтверждено', '2025-06-23 09:24:31.713934', true);


--
-- TOC entry 5029 (class 0 OID 16534)
-- Dependencies: 227
-- Data for Name: test_drives; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- TOC entry 5063 (class 0 OID 0)
-- Dependencies: 218
-- Name: admins_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.admins_id_seq', 3, true);


--
-- TOC entry 5064 (class 0 OID 0)
-- Dependencies: 220
-- Name: car_types_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.car_types_id_seq', 6, true);


--
-- TOC entry 5065 (class 0 OID 0)
-- Dependencies: 222
-- Name: cars_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.cars_id_seq', 76, true);


--
-- TOC entry 5066 (class 0 OID 0)
-- Dependencies: 224
-- Name: clients_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.clients_id_seq', 7, true);


--
-- TOC entry 5067 (class 0 OID 0)
-- Dependencies: 232
-- Name: insurance_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.insurance_requests_id_seq', 14, true);


--
-- TOC entry 5068 (class 0 OID 0)
-- Dependencies: 230
-- Name: loan_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.loan_requests_id_seq', 6, true);


--
-- TOC entry 5069 (class 0 OID 0)
-- Dependencies: 226
-- Name: purchases_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.purchases_id_seq', 23, true);


--
-- TOC entry 5070 (class 0 OID 0)
-- Dependencies: 235
-- Name: rental_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.rental_requests_id_seq', 7, true);


--
-- TOC entry 5071 (class 0 OID 0)
-- Dependencies: 234
-- Name: service_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.service_requests_id_seq', 7, true);


--
-- TOC entry 5072 (class 0 OID 0)
-- Dependencies: 228
-- Name: test_drives_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.test_drives_id_seq', 1, true);


--
-- TOC entry 4830 (class 2606 OID 16504)
-- Name: admins admins_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_pkey PRIMARY KEY (id);


--
-- TOC entry 4832 (class 2606 OID 16506)
-- Name: admins admins_username_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_username_key UNIQUE (username);


--
-- TOC entry 4834 (class 2606 OID 16508)
-- Name: car_types car_types_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types
    ADD CONSTRAINT car_types_pkey PRIMARY KEY (id);


--
-- TOC entry 4836 (class 2606 OID 16510)
-- Name: cars cars_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT cars_pkey PRIMARY KEY (id);


--
-- TOC entry 4838 (class 2606 OID 16512)
-- Name: clients clients_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_email_key UNIQUE (email);


--
-- TOC entry 4840 (class 2606 OID 16514)
-- Name: clients clients_phone_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_phone_key UNIQUE (phone);


--
-- TOC entry 4842 (class 2606 OID 16516)
-- Name: clients clients_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_pkey PRIMARY KEY (id);


--
-- TOC entry 4850 (class 2606 OID 16600)
-- Name: insurance_requests insurance_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4848 (class 2606 OID 16579)
-- Name: loan_requests loan_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4844 (class 2606 OID 16518)
-- Name: purchases purchases_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_pkey PRIMARY KEY (id);


--
-- TOC entry 4854 (class 2606 OID 49329)
-- Name: rental_requests rental_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rental_requests
    ADD CONSTRAINT rental_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4852 (class 2606 OID 16620)
-- Name: service_requests service_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4846 (class 2606 OID 16541)
-- Name: test_drives test_drives_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_pkey PRIMARY KEY (id);


--
-- TOC entry 4870 (class 2620 OID 49314)
-- Name: insurance_requests handle_insurance_approval; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER handle_insurance_approval AFTER UPDATE ON public.insurance_requests FOR EACH ROW EXECUTE FUNCTION public.handle_approved_insurance_request();


--
-- TOC entry 4868 (class 2620 OID 49312)
-- Name: loan_requests handle_loan_approval; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER handle_loan_approval AFTER UPDATE ON public.loan_requests FOR EACH ROW EXECUTE FUNCTION public.handle_approved_loan_request();


--
-- TOC entry 4871 (class 2620 OID 49308)
-- Name: insurance_requests update_insurance_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER update_insurance_notification BEFORE UPDATE ON public.insurance_requests FOR EACH ROW WHEN ((old.notification_shown = false)) EXECUTE FUNCTION public.update_notification_status();


--
-- TOC entry 4869 (class 2620 OID 49309)
-- Name: loan_requests update_loan_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER update_loan_notification BEFORE UPDATE ON public.loan_requests FOR EACH ROW WHEN ((old.notification_shown = false)) EXECUTE FUNCTION public.update_notification_status();


--
-- TOC entry 4873 (class 2620 OID 49340)
-- Name: rental_requests update_rental_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER update_rental_notification BEFORE UPDATE ON public.rental_requests FOR EACH ROW WHEN ((old.notification_shown = false)) EXECUTE FUNCTION public.update_notification_status();


--
-- TOC entry 4872 (class 2620 OID 49310)
-- Name: service_requests update_service_notification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER update_service_notification BEFORE UPDATE ON public.service_requests FOR EACH ROW WHEN ((old.notification_shown = false)) EXECUTE FUNCTION public.update_notification_status();


--
-- TOC entry 4855 (class 2606 OID 16519)
-- Name: cars fk_type; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT fk_type FOREIGN KEY (type_id) REFERENCES public.car_types(id) ON DELETE CASCADE;


--
-- TOC entry 4862 (class 2606 OID 16606)
-- Name: insurance_requests insurance_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4863 (class 2606 OID 16601)
-- Name: insurance_requests insurance_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4860 (class 2606 OID 16585)
-- Name: loan_requests loan_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4861 (class 2606 OID 16580)
-- Name: loan_requests loan_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4856 (class 2606 OID 16524)
-- Name: purchases purchases_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4857 (class 2606 OID 16529)
-- Name: purchases purchases_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4866 (class 2606 OID 49335)
-- Name: rental_requests rental_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rental_requests
    ADD CONSTRAINT rental_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4867 (class 2606 OID 49330)
-- Name: rental_requests rental_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.rental_requests
    ADD CONSTRAINT rental_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4864 (class 2606 OID 16626)
-- Name: service_requests service_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4865 (class 2606 OID 16621)
-- Name: service_requests service_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4858 (class 2606 OID 16547)
-- Name: test_drives test_drives_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4859 (class 2606 OID 16542)
-- Name: test_drives test_drives_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


-- Completed on 2025-07-12 09:48:37

--
-- PostgreSQL database dump complete
--

