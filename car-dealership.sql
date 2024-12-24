--
-- PostgreSQL database dump
--

-- Dumped from database version 17.2
-- Dumped by pg_dump version 17.2

-- Started on 2024-12-24 16:03:23

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
-- TOC entry 222 (class 1259 OID 25583)
-- Name: admins; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.admins (
    id integer NOT NULL,
    username character varying(100) NOT NULL,
    password text NOT NULL
);


ALTER TABLE public.admins OWNER TO postgres;

--
-- TOC entry 221 (class 1259 OID 25582)
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
-- TOC entry 4900 (class 0 OID 0)
-- Dependencies: 221
-- Name: admins_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.admins_id_seq OWNED BY public.admins.id;


--
-- TOC entry 226 (class 1259 OID 25612)
-- Name: car_types; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.car_types (
    id integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.car_types OWNER TO postgres;

--
-- TOC entry 225 (class 1259 OID 25611)
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
-- TOC entry 4901 (class 0 OID 0)
-- Dependencies: 225
-- Name: car_types_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.car_types_id_seq OWNED BY public.car_types.id;


--
-- TOC entry 218 (class 1259 OID 25561)
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
-- TOC entry 217 (class 1259 OID 25560)
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
-- TOC entry 4902 (class 0 OID 0)
-- Dependencies: 217
-- Name: cars_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.cars_id_seq OWNED BY public.cars.id;


--
-- TOC entry 220 (class 1259 OID 25570)
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
-- TOC entry 219 (class 1259 OID 25569)
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
-- TOC entry 4903 (class 0 OID 0)
-- Dependencies: 219
-- Name: clients_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.clients_id_seq OWNED BY public.clients.id;


--
-- TOC entry 224 (class 1259 OID 25594)
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
-- TOC entry 223 (class 1259 OID 25593)
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
-- TOC entry 4904 (class 0 OID 0)
-- Dependencies: 223
-- Name: purchases_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.purchases_id_seq OWNED BY public.purchases.id;


--
-- TOC entry 4717 (class 2604 OID 25586)
-- Name: admins id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins ALTER COLUMN id SET DEFAULT nextval('public.admins_id_seq'::regclass);


--
-- TOC entry 4720 (class 2604 OID 25615)
-- Name: car_types id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types ALTER COLUMN id SET DEFAULT nextval('public.car_types_id_seq'::regclass);


--
-- TOC entry 4715 (class 2604 OID 25564)
-- Name: cars id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars ALTER COLUMN id SET DEFAULT nextval('public.cars_id_seq'::regclass);


--
-- TOC entry 4716 (class 2604 OID 25573)
-- Name: clients id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients ALTER COLUMN id SET DEFAULT nextval('public.clients_id_seq'::regclass);


--
-- TOC entry 4718 (class 2604 OID 25597)
-- Name: purchases id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases ALTER COLUMN id SET DEFAULT nextval('public.purchases_id_seq'::regclass);


--
-- TOC entry 4890 (class 0 OID 25583)
-- Dependencies: 222
-- Data for Name: admins; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.admins VALUES (1, 'admin1', 'adminpass1');
INSERT INTO public.admins VALUES (2, 'admin2', 'adminpass2');
INSERT INTO public.admins VALUES (3, 'admin3', 'adminpass3');


--
-- TOC entry 4894 (class 0 OID 25612)
-- Dependencies: 226
-- Data for Name: car_types; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.car_types VALUES (1, 'Лимузин');
INSERT INTO public.car_types VALUES (2, 'Внедорожник');
INSERT INTO public.car_types VALUES (3, 'Купе');
INSERT INTO public.car_types VALUES (4, 'Кабриолет');


--
-- TOC entry 4886 (class 0 OID 25561)
-- Dependencies: 218
-- Data for Name: cars; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.cars VALUES (59, 'GT 63 S 4MATIC+', 'Чёрный', 8420000, NULL, 'Mercedes-AMG GT 63 S 4MATIC+\black.png', 3);
INSERT INTO public.cars VALUES (60, 'GT 63 S 4MATIC+', 'Зелёный', 8420000, NULL, 'Mercedes-AMG GT 63 S 4MATIC+\green.png', 3);
INSERT INTO public.cars VALUES (61, 'GT 63 S 4MATIC+', 'Красный', 8420000, NULL, 'Mercedes-AMG GT 63 S 4MATIC+\red.png', 3);
INSERT INTO public.cars VALUES (62, 'GT 63 S 4MATIC+', 'Белый', 8420000, NULL, 'Mercedes-AMG GT 63 S 4MATIC+\white.png', 3);
INSERT INTO public.cars VALUES (63, 'GT 63 S 4MATIC+', 'Жёлтый', 8420000, NULL, 'Mercedes-AMG GT 63 S 4MATIC+\yellow.png', 3);
INSERT INTO public.cars VALUES (1, 'CLE 200 Cabriolet', 'Чёрный', 5683000, NULL, 'CLE 200 Cabriolet\black.png', 4);
INSERT INTO public.cars VALUES (2, 'CLE 200 Cabriolet', 'Голубой', 5683000, 'Compact car with great fuel efficiency.', 'CLE 200 Cabriolet\blue.png', 4);
INSERT INTO public.cars VALUES (3, 'CLE 200 Cabriolet', 'Красный', 5683000, 'Powerful full-size pickup truck.', 'CLE 200 Cabriolet\red.png', 4);
INSERT INTO public.cars VALUES (4, 'CLE 200 Cabriolet', 'Белый', 5683000, 'Luxury sports sedan.', 'CLE 200 Cabriolet\white.png', 4);
INSERT INTO public.cars VALUES (5, 'S 63 E Performance', 'Золотой магнолитет', 20520000, 'Electric vehicle with advanced technology.', 'Mercedes-AMG S 63 E Performance\kalaharigold magno.png', 1);
INSERT INTO public.cars VALUES (6, 'S 63 E Performance', 'Мистический чистый', 20520000, 'Reliable and stylish midsize car.', 'Mercedes-AMG S 63 E Performance\mysticblau metallic.png', 1);
INSERT INTO public.cars VALUES (7, 'S 63 E Performance', 'Ночной магно', 20520000, 'Скорость.', 'Mercedes-AMG S 63 E Performance\nachtschwarz magno.png', 1);
INSERT INTO public.cars VALUES (9, 'S 63 E Performance', 'Белый', 20520000, NULL, 'Mercedes-AMG S 63 E Performance\opalithwei bright.png', 1);
INSERT INTO public.cars VALUES (10, 'S 63 E Performance', 'Винтажно-голубой', 20520000, NULL, 'Mercedes-AMG S 63 E Performance\vintageblau uni.png', 1);
INSERT INTO public.cars VALUES (12, 'EQS 53 4MATIC+', 'Белый', 15700000, NULL, 'Mercedes-AMG EQS 53 4MATIC+\white.png', 1);
INSERT INTO public.cars VALUES (13, 'EQS 53 4MATIC+', 'Красный', 15700000, NULL, 'Mercedes-AMG EQS 53 4MATIC+\red.png', 1);
INSERT INTO public.cars VALUES (14, 'EQS 53 4MATIC+', 'Чёрный', 15700000, NULL, 'Mercedes-AMG EQS 53 4MATIC+\black.png', 1);
INSERT INTO public.cars VALUES (15, 'Mercedes-AMG G 63', 'Бриллиант', 19890000, NULL, 'Mercedes-AMG G 63\blue.png', 2);
INSERT INTO public.cars VALUES (16, 'Mercedes-AMG G 63', 'Белый', 19890000, NULL, 'Mercedes-AMG G 63\white.png', 2);
INSERT INTO public.cars VALUES (17, 'Mercedes-AMG G 63', 'Серый', 19890000, NULL, 'Mercedes-AMG G 63\gray.png', 2);
INSERT INTO public.cars VALUES (18, 'Mercedes-AMG G 63', 'Зелёный', 19890000, NULL, 'Mercedes-AMG G 63\green.png', 2);
INSERT INTO public.cars VALUES (19, 'Mercedes-AMG G 63', 'Красный', 19890000, NULL, 'Mercedes-AMG G 63\red.png', 2);
INSERT INTO public.cars VALUES (20, 'Mercedes-AMG G 63', 'Жёлтый', 19890000, NULL, 'Mercedes-AMG G 63\yellow.png', 2);
INSERT INTO public.cars VALUES (21, 'Mercedes-AMG GLB 35 4MATIC', 'Чёрный', 6840000, NULL, 'Mercedes-AMG GLB 35 4MATIC\black.png', 2);
INSERT INTO public.cars VALUES (22, 'Mercedes-AMG GLB 35 4MATIC', 'Синий', 6840000, NULL, 'Mercedes-AMG GLB 35 4MATIC\blue.png', 2);
INSERT INTO public.cars VALUES (23, 'Mercedes-AMG GLB 35 4MATIC', 'Красный', 6840000, NULL, 'Mercedes-AMG GLB 35 4MATIC\red.png', 2);
INSERT INTO public.cars VALUES (24, 'Mercedes-AMG GLB 35 4MATIC', 'Белый', 6840000, NULL, 'Mercedes-AMG GLB 35 4MATIC\white.png', 2);
INSERT INTO public.cars VALUES (25, 'GLC 43 4MATIC Coupé', 'Чёрный', 8840000, NULL, 'Mercedes-AMG GLC 43 4MATIC Coupé\black.png', 3);
INSERT INTO public.cars VALUES (26, 'GLC 43 4MATIC Coupé', 'Голубой', 8840000, NULL, 'Mercedes-AMG GLC 43 4MATIC Coupé\blue.png', 3);
INSERT INTO public.cars VALUES (27, 'GLC 43 4MATIC Coupé', 'Красный', 8840000, NULL, 'Mercedes-AMG GLC 43 4MATIC Coupé\red.png', 3);
INSERT INTO public.cars VALUES (28, 'GLC 43 4MATIC Coupé', 'Белый', 8840000, NULL, 'Mercedes-AMG GLC 43 4MATIC Coupé\white.png', 3);
INSERT INTO public.cars VALUES (38, 'GLS 63 4MATIC+', 'Чёрный', 18495000, NULL, 'Mercedes-AMG GLS 63 4MATIC+\black.png', 2);
INSERT INTO public.cars VALUES (39, 'GLS 63 4MATIC+', 'Голубой', 18495000, NULL, 'Mercedes-AMG GLS 63 4MATIC+\blue.png', 2);
INSERT INTO public.cars VALUES (40, 'GLS 63 4MATIC+', 'Зелёный', 18495000, NULL, 'Mercedes-AMG GLS 63 4MATIC+\green.png', 2);
INSERT INTO public.cars VALUES (41, 'GLS 63 4MATIC+', 'Красный', 18495000, NULL, 'Mercedes-AMG GLS 63 4MATIC+\red.png', 2);
INSERT INTO public.cars VALUES (42, 'GLS 63 4MATIC+', 'Серый', 18495000, NULL, 'Mercedes-AMG GLS 63 4MATIC+\gray.png', 2);
INSERT INTO public.cars VALUES (43, 'Mercedes-AMG SL 43', 'Чёрный', 5563000, NULL, 'Mercedes-AMG SL 43\black.png', 4);
INSERT INTO public.cars VALUES (44, 'Mercedes-AMG SL 43', 'Голубой', 5563000, NULL, 'Mercedes-AMG SL 43\blue.png', 4);
INSERT INTO public.cars VALUES (45, 'Mercedes-AMG SL 43', 'Красный', 5563000, NULL, 'Mercedes-AMG SL 43\red.png', 4);
INSERT INTO public.cars VALUES (46, 'Mercedes-AMG SL 43', 'Белый', 5563000, NULL, 'Mercedes-AMG SL 43\white.png', 4);
INSERT INTO public.cars VALUES (47, 'Mercedes-AMG SL 43', 'Жёлтый', 5563000, NULL, 'Mercedes-AMG SL 43\yellow.png', 4);
INSERT INTO public.cars VALUES (48, 'Mercedes-AMG GT 43', 'Чёрный', 12209000, NULL, 'Mercedes-AMG GT 43\black.png', 3);
INSERT INTO public.cars VALUES (49, 'Mercedes-AMG GT 43', 'Серый', 12209000, NULL, 'Mercedes-AMG GT 43\gray.png', 3);
INSERT INTO public.cars VALUES (50, 'Mercedes-AMG GT 43', 'Зелёный', 12209000, NULL, 'Mercedes-AMG GT 43\green.png', 3);
INSERT INTO public.cars VALUES (51, 'Mercedes-AMG GT 43', 'Голубой', 12209000, NULL, 'Mercedes-AMG GT 43\hyperblau.png', 3);
INSERT INTO public.cars VALUES (52, 'Mercedes-AMG GT 43', 'Красный', 12209000, NULL, 'Mercedes-AMG GT 43\red.png', 3);
INSERT INTO public.cars VALUES (53, 'Mercedes-AMG GT 43', 'Белый', 12209000, NULL, 'Mercedes-AMG GT 43\white.png', 3);
INSERT INTO public.cars VALUES (64, 'CLE 53 4MATIC+ Coupé', 'Чёрный', 9051000, NULL, 'Mercedes-AMG CLE 53 4MATIC+ Coupé\black.png', 3);
INSERT INTO public.cars VALUES (65, 'CLE 53 4MATIC+ Coupé', 'Красный', 9051000, NULL, 'Mercedes-AMG CLE 53 4MATIC+ Coupé\red.png', 3);
INSERT INTO public.cars VALUES (66, 'CLE 53 4MATIC+ Coupé', 'Белый', 9051000, NULL, 'Mercedes-AMG CLE 53 4MATIC+ Coupé\white.png', 3);
INSERT INTO public.cars VALUES (67, 'CLE 53 4MATIC+ Coupé', 'Жёлтый', 9051000, NULL, 'Mercedes-AMG CLE 53 4MATIC+ Coupé\yellow.png', 3);
INSERT INTO public.cars VALUES (68, 'GLS 63 4MATIC+', 'Белый', 18495000, NULL, 'Mercedes-AMG GLS 63 4MATIC+\white.png', 2);


--
-- TOC entry 4888 (class 0 OID 25570)
-- Dependencies: 220
-- Data for Name: clients; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.clients VALUES (1, 'John', 'Doe', '1234567890', 'john.doe@example.com', 'password123');
INSERT INTO public.clients VALUES (2, 'Jane', 'Smith', '0987654321', 'jane.smith@example.com', 'securepassword');
INSERT INTO public.clients VALUES (3, 'Alice', 'Johnson', '5678901234', 'alice.johnson@example.com', 'mypassword');
INSERT INTO public.clients VALUES (4, 'Bob', 'Brown', '3456789012', 'bob.brown@example.com', 'password321');
INSERT INTO public.clients VALUES (5, 'Charlie', 'Davis', '6789012345', 'charlie.davis@example.com', 'pass1234');
INSERT INTO public.clients VALUES (6, 'Emily', 'Wilson', '2345678901', 'emily.wilson@example.com', '1234secure');


--
-- TOC entry 4892 (class 0 OID 25594)
-- Dependencies: 224
-- Data for Name: purchases; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.purchases VALUES (2, 2, 2, '2024-12-23 13:45:23.764934');
INSERT INTO public.purchases VALUES (3, 3, 3, '2024-12-23 13:45:23.764934');
INSERT INTO public.purchases VALUES (4, 4, 4, '2024-12-23 13:45:23.764934');
INSERT INTO public.purchases VALUES (5, 5, 5, '2024-12-23 13:45:23.764934');
INSERT INTO public.purchases VALUES (6, 6, 6, '2024-12-23 13:45:23.764934');
INSERT INTO public.purchases VALUES (7, 68, 1, '2024-12-24 14:40:23.064912');
INSERT INTO public.purchases VALUES (8, 16, 1, '2024-12-24 15:07:52.338145');
INSERT INTO public.purchases VALUES (9, 41, 1, '2024-12-24 15:15:06.965101');
INSERT INTO public.purchases VALUES (10, 47, 1, '2024-12-24 15:15:20.385586');


--
-- TOC entry 4905 (class 0 OID 0)
-- Dependencies: 221
-- Name: admins_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.admins_id_seq', 3, true);


--
-- TOC entry 4906 (class 0 OID 0)
-- Dependencies: 225
-- Name: car_types_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.car_types_id_seq', 6, true);


--
-- TOC entry 4907 (class 0 OID 0)
-- Dependencies: 217
-- Name: cars_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.cars_id_seq', 68, true);


--
-- TOC entry 4908 (class 0 OID 0)
-- Dependencies: 219
-- Name: clients_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.clients_id_seq', 6, true);


--
-- TOC entry 4909 (class 0 OID 0)
-- Dependencies: 223
-- Name: purchases_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.purchases_id_seq', 11, true);


--
-- TOC entry 4730 (class 2606 OID 25590)
-- Name: admins admins_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_pkey PRIMARY KEY (id);


--
-- TOC entry 4732 (class 2606 OID 25592)
-- Name: admins admins_username_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_username_key UNIQUE (username);


--
-- TOC entry 4736 (class 2606 OID 25619)
-- Name: car_types car_types_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.car_types
    ADD CONSTRAINT car_types_pkey PRIMARY KEY (id);


--
-- TOC entry 4722 (class 2606 OID 25568)
-- Name: cars cars_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT cars_pkey PRIMARY KEY (id);


--
-- TOC entry 4724 (class 2606 OID 25581)
-- Name: clients clients_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_email_key UNIQUE (email);


--
-- TOC entry 4726 (class 2606 OID 25579)
-- Name: clients clients_phone_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_phone_key UNIQUE (phone);


--
-- TOC entry 4728 (class 2606 OID 25577)
-- Name: clients clients_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_pkey PRIMARY KEY (id);


--
-- TOC entry 4734 (class 2606 OID 25600)
-- Name: purchases purchases_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_pkey PRIMARY KEY (id);


--
-- TOC entry 4737 (class 2606 OID 25620)
-- Name: cars fk_type; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cars
    ADD CONSTRAINT fk_type FOREIGN KEY (type_id) REFERENCES public.car_types(id) ON DELETE CASCADE;


--
-- TOC entry 4738 (class 2606 OID 25601)
-- Name: purchases purchases_car_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_car_id_fkey FOREIGN KEY (car_id) REFERENCES public.cars(id) ON DELETE CASCADE;


--
-- TOC entry 4739 (class 2606 OID 25606)
-- Name: purchases purchases_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


-- Completed on 2024-12-24 16:03:23

--
-- PostgreSQL database dump complete
--

