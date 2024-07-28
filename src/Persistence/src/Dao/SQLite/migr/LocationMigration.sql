@migr(id = "c5d3e046-9efb-4f65-ab77-2981c06ef3f8", descn = "Create location type enumeration table", step_cnt = 2)
create table enum_location_type(
    id integer primary key,
    sym_id text not null,
    name text,
    desc text
);
create unique index enum_location_type_idx1 on enum_location_type(sym_id);

@migr(id = "c5d3e046-9efb-4f65-ab77-2981c06ef3f8", descn = "Insert location types", step = 2)
insert into enum_location_type(sym_id, name, desc)
values
  ('S', 'System', 'Default locations provided by the system (Sky Dolly)'),
  ('I', 'Imported', 'Locations imported by a location import plugin'),
  ('U', 'User', 'User defined locations');

@migr(id = "ccf35c93-6d55-49c6-83ff-c35bb8468044", descn = "Create location category enumeration table", step_cnt = 2)
create table enum_location_category(
    id integer primary key,
    sym_id text not null,
    name text,
    desc text
);
create unique index enum_location_category_idx1 on enum_location_category(sym_id);

@migr(id = "ccf35c93-6d55-49c6-83ff-c35bb8468044", descn = "Insert location categories", step = 2)
insert into enum_location_category(sym_id, name, desc)
values
  ('00', 'No category', 'Uncategorised'),
  ('AP', 'Airport', 'Airport'),
  ('AS', 'Airstrip', 'Airstrip'),
  ('BR', 'Bridge', 'Bridge'),
  ('BU', 'Building', 'Building'),
  ('CA', 'Canyon', 'Canyon'),
  ('CI', 'City', 'City'),
  ('DA', 'Dam', 'Dam'),
  ('DE', 'Desert', 'Desert'),
  ('GL', 'Glacier', 'Glacier'),
  ('HP', 'Helipad', 'Helipad'),
  ('IS', 'Island', 'Island'),
  ('LA', 'Lake', 'Lake'),
  ('LM', 'Landmark', 'Landmark'),
  ('LH', 'Lighthouse', 'Lighthouse'),
  ('MO', 'Mountain', 'Mountain'),
  ('OT', 'Other', 'Other category'),
  ('PA', 'Park', 'Park'),
  ('PO', 'POI', 'Point of interest'),
  ('SE', 'Sea', 'Sea'),
  ('SP', 'Seaport', 'Seaport'),
  ('TO', 'Town', 'Town'),
  ('RI', 'River', 'River'),
  ('VA', 'Valley', 'Valley'),
  ('WA', 'Waterfall', 'Waterfall');

@migr(id = "38e58cbe-0e0c-47d5-88ae-1bd7295d15cc", descn = "Create country enumeration table", step_cnt = 2)
create table enum_country(
    id integer primary key,
    sym_id text not null,
    name text,
    desc text
);
create unique index enum_country_idx1 on enum_country(sym_id);

@migr(id = "38e58cbe-0e0c-47d5-88ae-1bd7295d15cc", descn = "Insert countries", step = 2)
insert into enum_country(sym_id, name)
values
  ('00', 'World'),
  ('AF', 'Afghanistan'),
  ('AX', 'Åland Islands'),
  ('AL', 'Albania'),
  ('DZ', 'Algeria'),
  ('AS', 'American Samoa'),
  ('AD', 'Andorra'),
  ('AO', 'Angola'),
  ('AI', 'Anguilla'),
  ('AQ', 'Antarctica'),
  ('AG', 'Antigua and Barbuda'),
  ('AR', 'Argentina'),
  ('AM', 'Armenia'),
  ('AW', 'Aruba'),
  ('AU', 'Australia'),
  ('AT', 'Austria'),
  ('AZ', 'Azerbaijan'),
  ('BS', 'Bahamas'),
  ('BH', 'Bahrain'),
  ('BD', 'Bangladesh'),
  ('BB', 'Barbados'),
  ('BY', 'Belarus'),
  ('BE', 'Belgium'),
  ('BZ', 'Belize'),
  ('BJ', 'Benin'),
  ('BM', 'Bermuda'),
  ('BT', 'Bhutan'),
  ('BO', 'Bolivia'),
  ('BQ', 'Bonaire, Sint Eustatius and Saba'),
  ('BA', 'Bosnia and Herzegovina'),
  ('BW', 'Botswana'),
  ('BV', 'Bouvet Island'),
  ('BR', 'Brazil'),
  ('IO', 'British Indian Ocean Territory'),
  ('BN', 'Brunei Darussalam'),
  ('BG', 'Bulgaria'),
  ('BF', 'Burkina Faso'),
  ('BI', 'Burundi'),
  ('CV', 'Cabo Verde'),
  ('KH', 'Cambodia'),
  ('CM', 'Cameroon'),
  ('CA', 'Canada'),
  ('KY', 'Cayman Islands'),
  ('CF', 'Central African Republic'),
  ('TD', 'Chad'),
  ('CL', 'Chile'),
  ('CN', 'China'),
  ('CX', 'Christmas Island'),
  ('CC', 'Cocos (Keeling) Islands'),
  ('CO', 'Columbia'),
  ('KM', 'Comoros'),
  ('CD', 'Congo (Democratic Republic)'),
  ('CG', 'Congo'),
  ('CK', 'Cook Islands'),
  ('CR', 'Costa Rica'),
  ('CI', 'Côte d''Ivoire'),
  ('HR', 'Croatia'),
  ('CU', 'Cuba'),
  ('CW', 'Curaçao'),
  ('CY', 'Cyprus'),
  ('CZ', 'Czechia'),
  ('DK', 'Denmark'),
  ('DJ', 'Djibouti'),
  ('DM', 'Dominica'),
  ('DO', 'Dominican Republic'),
  ('EC', 'Ecuador'),
  ('EG', 'Egypt'),
  ('SV', 'El Salvador'),
  ('GQ', 'Equatorial Guinea'),
  ('ER', 'Eritrea'),
  ('EE', 'Estonia'),
  ('SZ', 'Eswatini'),
  ('ET', 'Ethiopia'),
  ('FK', 'Falkland Islands [Malvinas]'),
  ('FO', 'Faroe Islands'),
  ('FJ', 'Fiji'),
  ('FI', 'Finland'),
  ('FR', 'France'),
  ('GF', 'French Guiana'),
  ('PF', 'French Polynesia'),
  ('TF', 'French Southern Territories'),
  ('GA', 'Gabon'),
  ('GM', 'Gambia'),
  ('GE', 'Georgia'),
  ('DE', 'Germany'),
  ('GH', 'Ghana'),
  ('GI', 'Gibraltar'),
  ('GR', 'Greece'),
  ('GL', 'Greenland'),
  ('GD', 'Grenada'),
  ('GP', 'Guadeloupe'),
  ('GU', 'Guam'),
  ('GT', 'Guatemala'),
  ('GG', 'Guernsey'),
  ('GN', 'Guinea'),
  ('GW', 'Guinea-Bissau'),
  ('GY', 'Guyana'),
  ('HT', 'Haiti'),
  ('HM', 'Heard Island and McDonald Islands'),
  ('VA', 'Holy See'),
  ('HN', 'Honduras'),
  ('HK', 'Hong Kong'),
  ('HU', 'Hungary'),
  ('IS', 'Iceland'),
  ('IN', 'India'),
  ('ID', 'Indonesia'),
  ('IR', 'Iran'),
  ('IQ', 'Iraq'),
  ('IE', 'Ireland'),
  ('IM', 'Isle of Man'),
  ('IL', 'Israel'),
  ('IT', 'Italy'),
  ('JM', 'Jamaica'),
  ('JP', 'Japan'),
  ('JE', 'Jersey'),
  ('JO', 'Jordan'),
  ('KZ', 'Kazakhstan'),
  ('KE', 'Kenya'),
  ('KI', 'Kiribati'),
  ('KP', 'Democratic People''s Republic of Korea'),
  ('KR', 'Republic of Korea'),
  ('KW', 'Kuwait'),
  ('KG', 'Kyrgyzstan'),
  ('LA', 'Lao People''s Democratic Republic'),
  ('LV', 'Latvia'),
  ('LB', 'Lebanon'),
  ('LS', 'Lesotho'),
  ('LR', 'Liberia'),
  ('LY', 'Libya'),
  ('LI', 'Liechtenstein'),
  ('LT', 'Lithuania'),
  ('LU', 'Luxembourg'),
  ('MO', 'Macao'),
  ('MK', 'North Macedonia'),
  ('MG', 'Madagascar'),
  ('MW', 'Malawi'),
  ('MY', 'Malaysia'),
  ('MV', 'Maldives'),
  ('ML', 'Mali'),
  ('MT', 'Malta'),
  ('MH', 'Marshall Islands'),
  ('MQ', 'Martinique'),
  ('MR', 'Mauritania'),
  ('MU', 'Mauritius'),
  ('YT', 'Mayotte'),
  ('MX', 'Mexico'),
  ('FM', 'Micronesia'),
  ('MD', 'Moldova'),
  ('MC', 'Monaco'),
  ('MN', 'Mongolia'),
  ('ME', 'Montenegro'),
  ('MS', 'Montserrat'),
  ('MA', 'Morocco'),
  ('MZ', 'Mozambique'),
  ('MM', 'Myanmar'),
  ('NA', 'Namibia'),
  ('NR', 'Nauru'),
  ('NP', 'Nepal'),
  ('NL', 'Netherlands'),
  ('NC', 'New Caledonia'),
  ('NZ', 'New Zealand'),
  ('NI', 'Nicaragua'),
  ('NE', 'Niger'),
  ('NG', 'Nigeria'),
  ('NU', 'Niue'),
  ('NF', 'Norfolk Island'),
  ('MP', 'Northern Mariana Islands'),
  ('NO', 'Norway'),
  ('OM', 'Oman'),
  ('PK', 'Pakistan'),
  ('PW', 'Palau'),
  ('PS', 'Palestine'),
  ('PA', 'Panama'),
  ('PG', 'Papua New Guinea'),
  ('PY', 'Paraguay'),
  ('PE', 'Peru'),
  ('PH', 'Philippines'),
  ('PN', 'Pitcairn'),
  ('PL', 'Poland'),
  ('PT', 'Portugal'),
  ('PR', 'Puerto Rico'),
  ('QA', 'Quatar'),
  ('RE', 'Réunion'),
  ('RO', 'Romania'),
  ('RU', 'Russian Federation'),
  ('RW', 'Rwanda'),
  ('BL', 'Saint Barthélemy'),
  ('SH', 'Saint Helena, Ascension and Tristan da Cunha'),
  ('KN', 'Saint Kitts and Nevis'),
  ('LC', 'Saint Lucia'),
  ('MF', 'Saint Martin'),
  ('PM', 'Saint Pierre and Miquelon'),
  ('VC', 'Saint Vincent and the Grenadines'),
  ('WS', 'Samoa'),
  ('SM', 'San Marino'),
  ('ST', 'Sao Tome and Principe'),
  ('SA', 'Saudi Arabia'),
  ('SN', 'Senegal'),
  ('RS', 'Serbia'),
  ('SC', 'Seychelles'),
  ('SL', 'Sierra Leone'),
  ('SG', 'Singapore'),
  ('SX', 'Sint Maarten'),
  ('SK', 'Slovakia'),
  ('SI', 'Slovenia'),
  ('SB', 'Solomon Islands'),
  ('SO', 'Somalia'),
  ('ZA', 'South Africa'),
  ('GS', 'South Georgia and the South Sandwich Islands'),
  ('SS', 'South Sudan'),
  ('ES', 'Spain'),
  ('LK', 'Sri Lanka'),
  ('SD', 'Sudan'),
  ('SR', 'Suriname'),
  ('SJ', 'Svalbard and Jan Mayen'),
  ('SE', 'Sweden'),
  ('CH', 'Switzerland'),
  ('SY', 'Syrian Arab Republic'),
  ('TW', 'Taiwan'),
  ('TJ', 'Tajikistan'),
  ('TZ', 'Tanzania'),
  ('TH', 'Thailand'),
  ('TL', 'Timor-Leste'),
  ('TG', 'Togo'),
  ('TK', 'Tokelau'),
  ('TO', 'Tonga'),
  ('TT', 'Trinidad and Tobago'),
  ('TN', 'Tunisia'),
  ('TR', 'Türkiye'),
  ('TM', 'Turkmenistan'),
  ('TC', 'Turks and Caicos Islands'),
  ('TV', 'Tuvalu'),
  ('UG', 'Uganda'),
  ('UA', 'Ukraine'),
  ('AE', 'United Arab Emirates'),
  ('GB', 'United Kingdom of Great Britain and Northern Ireland'),
  ('UM', 'United States Minor Outlying Islands'),
  ('US', 'United States of America'),
  ('UY', 'Uruguay'),
  ('UZ', 'Uzbekistan'),
  ('VU', 'Vanuatu'),
  ('VE', 'Venezuela'),
  ('VN', 'Viet Nam'),
  ('VG', 'Virgin Islands (British)'),
  ('VI', 'Virgin Islands (U.S.)'),
  ('WF', 'Wallis and Futuna'),
  ('EH', 'Western Sahara'),
  ('YE', 'Yemen'),
  ('ZM', 'Zambia'),
  ('ZW', 'Zimbabwe');

@migr(id = "5f105246-8898-41aa-a292-073cce33a341", descn = "Create location table", step = 1)
create table location(
    id integer primary key,
    title text,
    description text,
    type_id integer,
    category_id integer,
    country_id integer,
    identifier text,
    latitude real,
    longitude real,
    altitude real,
    pitch real,
    bank real,
    true_heading real,
    indicated_airspeed integer,
    on_ground integer,
    attributes integer,
    foreign key(type_id) references enum_location_type(id)
    foreign key(category_id) references enum_location_category(id)
    foreign key(country_id) references enum_country(id)
);
create index location_idx1 on location(title collate nocase);
create index location_idx2 on location(description collate nocase);
create index location_idx3 on location(category_id);
create index location_idx4 on location(country_id);
create index location_idx5 on location(identifier collate nocase);
create index location_idx6 on location(on_ground);

@migr(id = "c94c121e-3d93-44e4-a747-6db2b5e3b45b", descn = "Update application version to 0.12", step = 1)
update metadata
set    app_version = '0.12.0';

@migr(id = "eeed782c-d594-4635-88d7-15b8d4ab7edc", descn = "Create engine event enumeration table", step_cnt = 2)
create table enum_engine_event(
    id integer primary key,
    sym_id text not null,
    name text,
    desc text
);
create unique index enum_engine_event_idx1 on enum_engine_event(sym_id);

@migr(id = "eeed782c-d594-4635-88d7-15b8d4ab7edc", descn = "Insert location engine events", step = 2)
insert into enum_engine_event(sym_id, name, desc)
values
  ('START', 'Start', 'Start the engine'),
  ('STOP', 'Stop', 'Stop the engine'),
  ('KEEP', 'Unchanged', 'Keep the engine state as is');

@migr(id = "db2ef8c9-8a62-47b5-96d1-37146404f51e", descn = "Add engine event column", step_cnt = 2)
alter table location add column engine_event integer references enum_engine_event(id);

@migr(id = "db2ef8c9-8a62-47b5-96d1-37146404f51e", descn = "Migrate engine event column", step = 2)
update location
set    engine_event = (select ee.id
                       from enum_engine_event ee
                       where ee.sym_id = 'KEEP')
where  engine_event is null
  and  on_ground = 0;
update location
set    engine_event = (select ee.id
                       from enum_engine_event ee
                       where ee.sym_id = 'STOP')
where  engine_event is null
  and  on_ground = 1;

@migr(id = "d834e6da-efe9-4137-a1b8-ea85c7bb13cb", descn = "Insert water runway category", step = 1)
insert into enum_location_category(sym_id, name, desc)
values
  ('WR', 'Water runway', 'Water runway');

@migr(id = "6c11d4ef-f1a6-4ece-bc27-db4f0d8fa7d5", descn = "Correct country name", step = 1)
update enum_country
set    name = 'Qatar'
where  sym_id = 'QA';

@migr(id = "55a04d46-fc38-445a-8967-f84c96aa41bb", descn = "Update application version to 0.13", step = 1)
update metadata
set    app_version = '0.13.0';

@migr(id = "7620ca47-cbd3-4718-9ea9-0d701236aa58", descn = "Add new location categories", step = 1)
insert into enum_location_category(sym_id, name, desc)
values
  ('CB', 'Cabin', 'Cabin'),
  ('CR', 'Carrier', 'Carrier'),
  ('HI', 'History', 'History'),
  ('OB', 'Obstacle', 'Obstacle'),
  ('OP', 'Oil Platform', 'Oil platform'),
  ('ST', 'Settlement', 'Settlement');

@migr(id = "b7c620c9-5ffe-4fdc-a120-cba323e327d0", descn = "Update application version to 0.14", step = 1)
update metadata
set    app_version = '0.14.0';

@migr(id = "6a3f5879-8974-4685-9285-ec1f174fdacb", descn = "Update application version to 0.15", step = 1)
update metadata
set    app_version = '0.15.0';

@migr(id = "24916944-050e-4569-8352-1446fe3137d2", descn = "Update application version to 0.16", step = 1)
update metadata
set    app_version = '0.16.0';

@migr(id = "54eddc7a-1a89-40ae-af73-0c6ae74e6a2d", descn = "Update application version to 0.17", step = 1)
update metadata
set    app_version = '0.17.0';

@migr(id = "36bb8b21-76c4-4f71-9644-b78abaefdb64", descn = "Update application version to 0.18", step = 1)
update metadata
set    app_version = '0.18.0';

@migr(id = "b8eb79c7-476d-4ad4-9b6e-88698e03deaf", descn = "Add Kosovo to countries", step = 1)
insert into enum_country(sym_id, name)
values ('RS-KM', 'Kosovo');

@migr(id = "34200839-ddc4-4390-be0a-417ccd6aef2a", descn = "Adjust country names", step = 1)
update enum_country
set name = 'British Virgin Islands'
whre sym_id = 'VG';
update enum_country
set name = 'Brunei'
whre sym_id = 'BN';
update enum_country
set name = 'Cape Verde'
whre sym_id = 'CV';
update enum_country
set name = 'Colombia'
whre sym_id = 'CO';

@migr(id = "1b1fe05f-6b17-451e-9764-906baed01a98", descn = "Add beach to location categories", step = 1)
insert into enum_location_category(sym_id, name, desc)
values ('BE', 'Beach', 'Beach'),
       ('CS', 'Castle', 'Castle');

@migr(id = "37e34152-4f60-4f28-8c9a-7b2311b6819c", descn = "Update application version to 0.19", step = 1)
update metadata
set    app_version = '0.19.0';
