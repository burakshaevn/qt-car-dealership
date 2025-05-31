--
-- PostgreSQL database dump
--

-- Dumped from database version 17.4
-- Dumped by pg_dump version 17.4

-- Started on 2025-05-24 17:18:39

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
-- TOC entry 5025 (class 0 OID 0)
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
-- TOC entry 5026 (class 0 OID 0)
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
    type_id integer NOT NULL
);


ALTER TABLE public.cars OWNER TO postgres;

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
-- TOC entry 5027 (class 0 OID 0)
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
-- TOC entry 5028 (class 0 OID 0)
-- Dependencies: 224
-- Name: clients_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.clients_id_seq OWNED BY public.clients.id;


--
-- TOC entry 233 (class 1259 OID 16592)
-- Name: insurance_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.insurance_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    insurance_type character varying(50) NOT NULL,
    status character varying(20) DEFAULT 'не обработано'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT insurance_requests_insurance_type_check CHECK (((insurance_type)::text = ANY ((ARRAY['ОСАГО'::character varying, 'КАСКО'::character varying, 'Комплекс'::character varying])::text[]))),
    CONSTRAINT insurance_requests_status_check CHECK (((status)::text = ANY ((ARRAY['не обработано'::character varying, 'одобрено'::character varying, 'отклонено'::character varying, 'завершено'::character varying])::text[])))
);


ALTER TABLE public.insurance_requests OWNER TO postgres;

--
-- TOC entry 5029 (class 0 OID 0)
-- Dependencies: 233
-- Name: TABLE insurance_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.insurance_requests IS 'Страховые запросы.';


--
-- TOC entry 234 (class 1259 OID 16611)
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
-- TOC entry 5030 (class 0 OID 0)
-- Dependencies: 234
-- Name: insurance_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.insurance_requests_id_seq OWNED BY public.insurance_requests.id;


--
-- TOC entry 231 (class 1259 OID 16570)
-- Name: loan_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.loan_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_id integer NOT NULL,
    loan_amount numeric(15,0) NOT NULL,
    loan_term_months integer NOT NULL,
    status character varying(20) DEFAULT 'pending'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT loan_requests_loan_amount_check CHECK ((loan_amount > (0)::numeric)),
    CONSTRAINT loan_requests_loan_term_months_check CHECK ((loan_term_months > 0)),
    CONSTRAINT loan_requests_status_check CHECK (((status)::text = ANY ((ARRAY['pending'::character varying, 'approved'::character varying, 'rejected'::character varying, 'completed'::character varying])::text[])))
);


ALTER TABLE public.loan_requests OWNER TO postgres;

--
-- TOC entry 5031 (class 0 OID 0)
-- Dependencies: 231
-- Name: TABLE loan_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.loan_requests IS 'Запросы на кредитование.';


--
-- TOC entry 232 (class 1259 OID 16590)
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
-- TOC entry 5032 (class 0 OID 0)
-- Dependencies: 232
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
    purchase_date timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.purchases OWNER TO postgres;

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
-- TOC entry 5033 (class 0 OID 0)
-- Dependencies: 226
-- Name: purchases_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.purchases_id_seq OWNED BY public.purchases.id;


--
-- TOC entry 229 (class 1259 OID 16554)
-- Name: sell_requests; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.sell_requests (
    id integer NOT NULL,
    client_id integer NOT NULL,
    car_make character varying(100) NOT NULL,
    car_model character varying(100) NOT NULL,
    car_year integer NOT NULL,
    proposed_price numeric(15,0),
    status character varying(20) DEFAULT 'pending'::character varying NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT sell_requests_car_year_check CHECK (((car_year >= 1900) AND ((car_year)::numeric <= EXTRACT(year FROM CURRENT_DATE)))),
    CONSTRAINT sell_requests_status_check CHECK (((status)::text = ANY ((ARRAY['не обработано'::character varying, 'подтверждено'::character varying, 'выполнено'::character varying, 'отменено'::character varying])::text[])))
);


ALTER TABLE public.sell_requests OWNER TO postgres;

--
-- TOC entry 5034 (class 0 OID 0)
-- Dependencies: 229
-- Name: TABLE sell_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.sell_requests IS 'Запросы на продажу автомобилей автосалону.';


--
-- TOC entry 230 (class 1259 OID 16568)
-- Name: sell_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.sell_requests_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.sell_requests_id_seq OWNER TO postgres;

--
-- TOC entry 5035 (class 0 OID 0)
-- Dependencies: 230
-- Name: sell_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.sell_requests_id_seq OWNED BY public.sell_requests.id;


--
-- TOC entry 235 (class 1259 OID 16613)
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
    CONSTRAINT service_requests_status_check CHECK (((status)::text = ANY ((ARRAY['не обработано'::character varying, 'подтверждено'::character varying, 'выполнено'::character varying, 'отменено'::character varying])::text[])))
);


ALTER TABLE public.service_requests OWNER TO postgres;

--
-- TOC entry 5036 (class 0 OID 0)
-- Dependencies: 235
-- Name: TABLE service_requests; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.service_requests IS 'Запросы на дилерское обслуживание.';


--
-- TOC entry 236 (class 1259 OID 16631)
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
-- TOC entry 5037 (class 0 OID 0)
-- Dependencies: 236
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
    CONSTRAINT test_drives_status_check CHECK (((status)::text = ANY ((ARRAY['pending'::character varying, 'confirmed'::character varying, 'completed'::character varying, 'cancelled'::character varying])::text[])))
);


ALTER TABLE public.test_drives OWNER TO postgres;

--
-- TOC entry 5038 (class 0 OID 0)
-- Dependencies: 227
-- Name: TABLE test_drives; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.test_drives IS 'Запросы на тест-драйвы.';


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
-- TOC entry 5039 (class 0 OID 0)
-- Dependencies: 228
-- Name: test_drives_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.test_drives_id_seq OWNED BY public.test_drives.id;


--
-- TOC entry 4787 (class 2604 OID 16498)
-- Name: admins id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins ALTER COLUMN id SET DEFAULT nextval('public.admins_id_seq'::regclass);


--
-- TOC entry 4788 (class 2604 OID 16499)
-- Name: car_types id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types ALTER COLUMN id SET DEFAULT nextval('public.car_types_id_seq'::regclass);


--
-- TOC entry 4789 (class 2604 OID 16500)
-- Name: cars id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars ALTER COLUMN id SET DEFAULT nextval('public.cars_id_seq'::regclass);


--
-- TOC entry 4790 (class 2604 OID 16501)
-- Name: clients id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients ALTER COLUMN id SET DEFAULT nextval('public.clients_id_seq'::regclass);


--
-- TOC entry 4802 (class 2604 OID 16612)
-- Name: insurance_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests ALTER COLUMN id SET DEFAULT nextval('public.insurance_requests_id_seq'::regclass);


--
-- TOC entry 4799 (class 2604 OID 16591)
-- Name: loan_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests ALTER COLUMN id SET DEFAULT nextval('public.loan_requests_id_seq'::regclass);


--
-- TOC entry 4791 (class 2604 OID 16502)
-- Name: purchases id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases ALTER COLUMN id SET DEFAULT nextval('public.purchases_id_seq'::regclass);


--
-- TOC entry 4796 (class 2604 OID 16569)
-- Name: sell_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sell_requests ALTER COLUMN id SET DEFAULT nextval('public.sell_requests_id_seq'::regclass);


--
-- TOC entry 4805 (class 2604 OID 16632)
-- Name: service_requests id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests ALTER COLUMN id SET DEFAULT nextval('public.service_requests_id_seq'::regclass);


--
-- TOC entry 4793 (class 2604 OID 16553)
-- Name: test_drives id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives ALTER COLUMN id SET DEFAULT nextval('public.test_drives_id_seq'::regclass);


--
-- TOC entry 5000 (class 0 OID 16469)
-- Dependencies: 217
-- Data for Name: admins; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.admins (id, username, password) FROM stdin;
1	admin1	adminpass1
2	admin2	adminpass2
3	admin3	adminpass3
\.


--
-- TOC entry 5002 (class 0 OID 16475)
-- Dependencies: 219
-- Data for Name: car_types; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.car_types (id, name) FROM stdin;
1	Лимузин
2	Внедорожник
3	Купе
4	Кабриолет
\.


--
-- TOC entry 5004 (class 0 OID 16481)
-- Dependencies: 221
-- Data for Name: cars; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.cars (id, name, color, price, description, image_url, type_id) FROM stdin;
1	CLE 200 Cabriolet	Чёрный	5683000	Элегантный кабриолет с современным дизайном. Идеальный выбор для комфортных поездок по городу.	CLE 200 Cabriolet\\black.png	4
3	CLE 200 Cabriolet	Красный	5683000	Яркий красный кабриолет с спортивным характером. Вместительный салон и премиальная отделка.	CLE 200 Cabriolet\\red.png	4
4	CLE 200 Cabriolet	Белый	5683000	Белоснежный кабриолет с роскошным интерьером. Современные технологии и высокий уровень комфорта.	CLE 200 Cabriolet\\white.png	4
5	S 63 E Performance	Золотой магнолитет	20520000	Роскошный седан в эксклюзивном золотом цвете. Мощный гибридный двигатель и инновационные технологии.	Mercedes-AMG S 63 E Performance\\kalaharigold magno.png	1
2	CLE 200 Cabriolet	Синий	5683000	Стильный кабриолет в голубом цвете. Отличная динамика и топливная экономичность.	CLE 200 Cabriolet\\blue.png	4
6	S 63 E Performance	Синий	20520000	Изысканный седан в мистическом синем цвете. Превосходная управляемость и максимальный комфорт.	Mercedes-AMG S 63 E Performance\\mysticblau metallic.png	1
9	S 63 E Performance	Белый	20520000	Белоснежный представительский седан. Просторный салон с роскошной отделкой и передовыми системами.	Mercedes-AMG S 63 E Performance\\opalithwei bright.png	1
12	EQS 53 4MATIC+	Белый	15700000	Футуристичный электромобиль в белом цвете. Инновационный дизайн и выдающаяся дальность хода.	Mercedes-AMG EQS 53 4MATIC+\\white.png	1
13	EQS 53 4MATIC+	Красный	15700000	Динамичный электрокар в красном цвете. Мгновенный разгон и премиальное оснащение.	Mercedes-AMG EQS 53 4MATIC+\\red.png	1
14	EQS 53 4MATIC+	Чёрный	15700000	Элегантный чёрный электромобиль. Тихая и плавная езда с высоким уровнем комфорта.	Mercedes-AMG EQS 53 4MATIC+\\black.png	1
15	Mercedes-AMG G 63	Бриллиант	19890000	Легендарный внедорожник в бриллиантово-синем цвете. Непревзойдённая проходимость и мощный двигатель.	Mercedes-AMG G 63\\blue.png	2
16	Mercedes-AMG G 63	Белый	19890000	Классический белый внедорожник. Просторный салон и передовые системы безопасности.	Mercedes-AMG G 63\\white.png	2
17	Mercedes-AMG G 63	Серый	19890000	Серый внедорожник с брутальным дизайном. Отличная управляемость на любом покрытии.	Mercedes-AMG G 63\\gray.png	2
18	Mercedes-AMG G 63	Зелёный	19890000	Зелёный внедорожник для ценителей стиля. Мощный двигатель и комфортабельный интерьер.	Mercedes-AMG G 63\\green.png	2
19	Mercedes-AMG G 63	Красный	19890000	Яркий красный внедорожник с агрессивным дизайном. Впечатляющая динамика и проходимость.	Mercedes-AMG G 63\\red.png	2
20	Mercedes-AMG G 63	Жёлтый	19890000	Жёлтый внедорожник для смелых водителей. Уникальный внешний вид и выдающиеся характеристики.	Mercedes-AMG G 63\\yellow.png	2
21	Mercedes-AMG GLB 35 4MATIC	Чёрный	6840000	Компактный кроссовер в чёрном цвете. Универсальный автомобиль для города и путешествий.	Mercedes-AMG GLB 35 4MATIC\\black.png	2
22	Mercedes-AMG GLB 35 4MATIC	Синий	6840000	Стильный синий кроссовер с динамичным характером. Вместительный салон и экономичный двигатель.	Mercedes-AMG GLB 35 4MATIC\\blue.png	2
23	Mercedes-AMG GLB 35 4MATIC	Красный	6840000	Энергичный красный кроссовер. Отличная управляемость и современные технологии.	Mercedes-AMG GLB 35 4MATIC\\red.png	2
24	Mercedes-AMG GLB 35 4MATIC	Белый	6840000	Белый кроссовер с элегантным дизайном. Комфорт и практичность в одном автомобиле.	Mercedes-AMG GLB 35 4MATIC\\white.png	2
25	GLC 43 4MATIC Coupé	Чёрный	8840000	Спортивный купе-кроссовер в чёрном цвете. Динамичный дизайн и мощный двигатель.	Mercedes-AMG GLC 43 4MATIC Coupé\\black.png	3
27	GLC 43 4MATIC Coupé	Красный	8840000	Яркий красный купе-кроссовер. Спортивный характер и премиальное оснащение.	Mercedes-AMG GLC 43 4MATIC Coupé\\red.png	3
28	GLC 43 4MATIC Coupé	Белый	8840000	Белоснежный купе-кроссовер с роскошным интерьером. Комфорт и динамика в одном автомобиле.	Mercedes-AMG GLC 43 4MATIC Coupé\\white.png	3
38	GLS 63 4MATIC+	Чёрный	18495000	Большой чёрный внедорожник премиум-класса. Просторный салон и мощный двигатель.	Mercedes-AMG GLS 63 4MATIC+\\black.png	2
40	GLS 63 4MATIC+	Зелёный	18495000	Зелёный внедорожник для любителей природы. Вместительный и технологичный автомобиль.	Mercedes-AMG GLS 63 4MATIC+\\green.png	2
41	GLS 63 4MATIC+	Красный	18495000	Красный внедорожник с ярким характером. Мощность и роскошь в одном автомобиле.	Mercedes-AMG GLS 63 4MATIC+\\red.png	2
42	GLS 63 4MATIC+	Серый	18495000	Серый внедорожник с элегантным дизайном. Идеальный выбор для семьи и путешествий.	Mercedes-AMG GLS 63 4MATIC+\\gray.png	2
68	GLS 63 4MATIC+	Белый	18495000	Белый внедорожник премиум-класса. Просторный салон и передовые технологии.	Mercedes-AMG GLS 63 4MATIC+\\white.png	2
43	Mercedes-AMG SL 43	Чёрный	5563000	Роскошный чёрный родстер. Идеальный автомобиль для любителей открытых дорог.	Mercedes-AMG SL 43\\black.png	4
44	Mercedes-AMG SL 43	Голубой	5563000	Элегантный голубой родстер. Сочетание стиля и выдающихся динамических характеристик.	Mercedes-AMG SL 43\\blue.png	4
45	Mercedes-AMG SL 43	Красный	5563000	Страстный красный родстер. Спортивный характер и неповторимый дизайн.	Mercedes-AMG SL 43\\red.png	4
7	S 63 E Performance	Чёрный	20520000	Стильный чёрный седан с матовым покрытием. Впечатляющая мощность и динамичные характеристики.	Mercedes-AMG S 63 E Performance\\nachtschwarz magno.png	1
10	S 63 E Performance	Голубой	20520000	Эксклюзивный седан в винтажно-голубом цвете. Сочетание классического стиля и современных технологий.	Mercedes-AMG S 63 E Performance\\vintageblau uni.png	1
39	GLS 63 4MATIC+	Синий	18495000	Голубой внедорожник с представительским характером. Высокий уровень комфорта и безопасности.	Mercedes-AMG GLS 63 4MATIC+\\blue.png	2
46	Mercedes-AMG SL 43	Белый	5563000	Белоснежный родстер с премиальным интерьером. Комфорт и удовольствие от вождения.	Mercedes-AMG SL 43\\white.png	4
47	Mercedes-AMG SL 43	Жёлтый	5563000	Яркий жёлтый родстер для смелых водителей. Динамичный дизайн и мощный двигатель.	Mercedes-AMG SL 43\\yellow.png	4
48	Mercedes-AMG GT 43	Чёрный	12209000	Спортивный чёрный седан. Мощный двигатель и агрессивный дизайн.	Mercedes-AMG GT 43\\black.png	3
49	Mercedes-AMG GT 43	Серый	12209000	Элегантный серый седан с динамичным характером. Премиальное оснащение и комфорт.	Mercedes-AMG GT 43\\gray.png	3
50	Mercedes-AMG GT 43	Зелёный	12209000	Эксклюзивный зелёный седан. Уникальный внешний вид и выдающиеся характеристики.	Mercedes-AMG GT 43\\green.png	3
51	Mercedes-AMG GT 43	Голубой	12209000	Яркий голубой седан с футуристичным дизайном. Современные технологии и динамика.	Mercedes-AMG GT 43\\hyperblau.png	3
52	Mercedes-AMG GT 43	Красный	12209000	Страстный красный седан. Спортивный характер и роскошный интерьер.	Mercedes-AMG GT 43\\red.png	3
53	Mercedes-AMG GT 43	Белый	12209000	Белоснежный седан премиум-класса. Сочетание элегантности и мощности.	Mercedes-AMG GT 43\\white.png	3
59	GT 63 S 4MATIC+	Чёрный	8420000	Агрессивный чёрный спортивный автомобиль. Мощный двигатель и выдающаяся динамика.	Mercedes-AMG GT 63 S 4MATIC+\\black.png	3
60	GT 63 S 4MATIC+	Зелёный	8420000	Эксклюзивный зелёный спортивный автомобиль. Уникальный дизайн и премиальное оснащение.	Mercedes-AMG GT 63 S 4MATIC+\\green.png	3
61	GT 63 S 4MATIC+	Красный	8420000	Страстный красный спортивный автомобиль. Идеальный выбор для любителей скорости.	Mercedes-AMG GT 63 S 4MATIC+\\red.png	3
62	GT 63 S 4MATIC+	Белый	8420000	Элегантный белый спортивный автомобиль. Сочетание стиля и мощности.	Mercedes-AMG GT 63 S 4MATIC+\\white.png	3
63	GT 63 S 4MATIC+	Жёлтый	8420000	Яркий жёлтый спортивный автомобиль. Неповторимый внешний вид и динамичные характеристики.	Mercedes-AMG GT 63 S 4MATIC+\\yellow.png	3
64	CLE 53 4MATIC+ Coupé	Чёрный	9051000	Стильный чёрный купе. Динамичный дизайн и мощный двигатель.	Mercedes-AMG CLE 53 4MATIC+ Coupé\\black.png	3
65	CLE 53 4MATIC+ Coupé	Красный	9051000	Энергичный красный купе. Спортивный характер и современные технологии.	Mercedes-AMG CLE 53 4MATIC+ Coupé\\red.png	3
66	CLE 53 4MATIC+ Coupé	Белый	9051000	Элегантный белый купе. Роскошный интерьер и комфортабельный салон.	Mercedes-AMG CLE 53 4MATIC+ Coupé\\white.png	3
67	CLE 53 4MATIC+ Coupé	Жёлтый	9051000	Яркий жёлтый купе для смелых водителей. Уникальный внешний вид и выдающаяся динамика.	Mercedes-AMG CLE 53 4MATIC+ Coupé\\yellow.png	3
26	GLC 43 4MATIC Coupé	Синий	8840000	Элегантный голубой купе-кроссовер. Сочетание стиля и выдающихся ходовых качеств.	Mercedes-AMG GLC 43 4MATIC Coupé\\blue.png	3
69	E 220 d Limousine	Белый	6077570	\N	E 220 d Limousine\\white.png	1
70	E 220 d Limousine	Чёрный	6077570	\N	E 220 d Limousine\\black.png	1
71	E 220 d Limousine	Красный	6077570	\N	E 220 d Limousine\\red.png	1
72	Mercedes-Maybach SL 680 Monogram Series	Красный	24219570	\N	Mercedes-Maybach SL 680 Monogram Series\\red.png	4
73	Mercedes-Maybach SL 680 Monogram Series	Белый	24219570	\N	Mercedes-Maybach SL 680 Monogram Series\\white.png	4
74	Mercedes-Maybach SL 680 Monogram Series	Чёрный	24219570	\N	Mercedes-Maybach SL 680 Monogram Series\\black.png	4
75	Mercedes-Maybach SL 680 Monogram Series	Жёлтый	24219570	\N	Mercedes-Maybach SL 680 Monogram Series\\yellow.png	4
76	Mercedes-Maybach SL 680 Monogram Series	Синий	24219570	\N	Mercedes-Maybach SL 680 Monogram Series\\blue.png	4
\.


--
-- TOC entry 5006 (class 0 OID 16487)
-- Dependencies: 223
-- Data for Name: clients; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.clients (id, first_name, last_name, phone, email, password) FROM stdin;
2	Jane	Smith	0987654321	jane.smith@example.com	securepassword
3	Alice	Johnson	5678901234	alice.johnson@example.com	mypassword
4	Bob	Brown	3456789012	bob.brown@example.com	password321
5	Charlie	Davis	6789012345	charlie.davis@example.com	pass1234
6	Emily	Wilson	2345678901	emily.wilson@example.com	1234secure
1	Никита	Буракшаев	1234567890	nb@example.com	password123
\.


--
-- TOC entry 5016 (class 0 OID 16592)
-- Dependencies: 233
-- Data for Name: insurance_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.insurance_requests (id, client_id, car_id, insurance_type, status, created_at) FROM stdin;
6	1	1	ОСАГО	не обработано	2025-05-22 06:08:25.928695
\.


--
-- TOC entry 5014 (class 0 OID 16570)
-- Dependencies: 231
-- Data for Name: loan_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.loan_requests (id, client_id, car_id, loan_amount, loan_term_months, status, created_at) FROM stdin;
\.


--
-- TOC entry 5008 (class 0 OID 16493)
-- Dependencies: 225
-- Data for Name: purchases; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.purchases (id, car_id, client_id, purchase_date) FROM stdin;
2	2	2	2024-12-23 13:45:23.764934
3	3	3	2024-12-23 13:45:23.764934
4	4	4	2024-12-23 13:45:23.764934
5	5	5	2024-12-23 13:45:23.764934
6	6	6	2024-12-23 13:45:23.764934
8	16	1	2024-12-24 15:07:52.338145
\.


--
-- TOC entry 5012 (class 0 OID 16554)
-- Dependencies: 229
-- Data for Name: sell_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.sell_requests (id, client_id, car_make, car_model, car_year, proposed_price, status, created_at) FROM stdin;
\.


--
-- TOC entry 5018 (class 0 OID 16613)
-- Dependencies: 235
-- Data for Name: service_requests; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.service_requests (id, client_id, car_id, service_type, scheduled_date, status, created_at) FROM stdin;
5	1	1	Замена масла	2025-05-19 09:00:00	не обработано	2025-05-19 04:42:01.487039
\.


--
-- TOC entry 5010 (class 0 OID 16534)
-- Dependencies: 227
-- Data for Name: test_drives; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.test_drives (id, client_id, car_id, scheduled_date, status, created_at) FROM stdin;
\.


--
-- TOC entry 5040 (class 0 OID 0)
-- Dependencies: 218
-- Name: admins_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.admins_id_seq', 3, true);


--
-- TOC entry 5041 (class 0 OID 0)
-- Dependencies: 220
-- Name: car_types_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.car_types_id_seq', 6, true);


--
-- TOC entry 5042 (class 0 OID 0)
-- Dependencies: 222
-- Name: cars_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.cars_id_seq', 76, true);


--
-- TOC entry 5043 (class 0 OID 0)
-- Dependencies: 224
-- Name: clients_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.clients_id_seq', 6, true);


--
-- TOC entry 5044 (class 0 OID 0)
-- Dependencies: 234
-- Name: insurance_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.insurance_requests_id_seq', 6, true);


--
-- TOC entry 5045 (class 0 OID 0)
-- Dependencies: 232
-- Name: loan_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.loan_requests_id_seq', 1, false);


--
-- TOC entry 5046 (class 0 OID 0)
-- Dependencies: 226
-- Name: purchases_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.purchases_id_seq', 11, true);


--
-- TOC entry 5047 (class 0 OID 0)
-- Dependencies: 230
-- Name: sell_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.sell_requests_id_seq', 1, false);


--
-- TOC entry 5048 (class 0 OID 0)
-- Dependencies: 236
-- Name: service_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.service_requests_id_seq', 5, true);


--
-- TOC entry 5049 (class 0 OID 0)
-- Dependencies: 228
-- Name: test_drives_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.test_drives_id_seq', 1, false);


--
-- TOC entry 4818 (class 2606 OID 16504)
-- Name: admins admins_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_pkey PRIMARY KEY (id);


--
-- TOC entry 4820 (class 2606 OID 16506)
-- Name: admins admins_username_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_username_key UNIQUE (username);


--
-- TOC entry 4822 (class 2606 OID 16508)
-- Name: car_types car_types_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types
    ADD CONSTRAINT car_types_pkey PRIMARY KEY (id);


--
-- TOC entry 4824 (class 2606 OID 16510)
-- Name: cars cars_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT cars_pkey PRIMARY KEY (id);


--
-- TOC entry 4826 (class 2606 OID 16512)
-- Name: clients clients_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_email_key UNIQUE (email);


--
-- TOC entry 4828 (class 2606 OID 16514)
-- Name: clients clients_phone_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_phone_key UNIQUE (phone);


--
-- TOC entry 4830 (class 2606 OID 16516)
-- Name: clients clients_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_pkey PRIMARY KEY (id);


--
-- TOC entry 4840 (class 2606 OID 16600)
-- Name: insurance_requests insurance_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4838 (class 2606 OID 16579)
-- Name: loan_requests loan_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4832 (class 2606 OID 16518)
-- Name: purchases purchases_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_pkey PRIMARY KEY (id);


--
-- TOC entry 4836 (class 2606 OID 16562)
-- Name: sell_requests sell_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sell_requests
    ADD CONSTRAINT sell_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4842 (class 2606 OID 16620)
-- Name: service_requests service_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_pkey PRIMARY KEY (id);


--
-- TOC entry 4834 (class 2606 OID 16541)
-- Name: test_drives test_drives_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_pkey PRIMARY KEY (id);


--
-- TOC entry 4843 (class 2606 OID 16519)
-- Name: cars fk_type; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT fk_type FOREIGN KEY (type_id) REFERENCES public.car_types(id) ON DELETE CASCADE;


--
-- TOC entry 4851 (class 2606 OID 16606)
-- Name: insurance_requests insurance_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4852 (class 2606 OID 16601)
-- Name: insurance_requests insurance_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.insurance_requests
    ADD CONSTRAINT insurance_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4849 (class 2606 OID 16585)
-- Name: loan_requests loan_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4850 (class 2606 OID 16580)
-- Name: loan_requests loan_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.loan_requests
    ADD CONSTRAINT loan_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4844 (class 2606 OID 16524)
-- Name: purchases purchases_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4845 (class 2606 OID 16529)
-- Name: purchases purchases_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4848 (class 2606 OID 16563)
-- Name: sell_requests sell_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sell_requests
    ADD CONSTRAINT sell_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4853 (class 2606 OID 16626)
-- Name: service_requests service_requests_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4854 (class 2606 OID 16621)
-- Name: service_requests service_requests_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.service_requests
    ADD CONSTRAINT service_requests_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4846 (class 2606 OID 16547)
-- Name: test_drives test_drives_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4847 (class 2606 OID 16542)
-- Name: test_drives test_drives_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.test_drives
    ADD CONSTRAINT test_drives_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


-- Completed on 2025-05-24 17:18:40

--
-- PostgreSQL database dump complete
--

