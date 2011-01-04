/* 
 * File:   main.c
 * Author: swistak
 *
 * Created on 30 grudzień 2010, 21:07
 */

#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <time.h>
#include <malloc.h>
#include <libpqtypes.h>

/*
 SELECT
     a.attname as "Column",
     pg_catalog.format_type(a.atttypid, a.atttypmod) as "Datatype"
 FROM
     pg_catalog.pg_attribute a
 WHERE
     a.attnum > 0
     AND NOT a.attisdropped
     AND a.attrelid = (
         SELECT c.oid
         FROM pg_catalog.pg_class c
             LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
         WHERE c.relname ~ '^(event)$'
             AND pg_catalog.pg_table_is_visible(c.oid)
     )
 ;
 */


void generate_test_data() {
    ca_execute("BEGIN;");

    ca_execute(
            "CREATE OR REPLACE FUNCTION random_numeric_string(l integer) RETURNS varchar AS $$"
            " SELECT array_to_string(ARRAY(SELECT chr((48 + round(random() * 9 )) :: integer) FROM generate_series(1,$1)), '') "
            "$$ LANGUAGE sql VOLATILE;"
            );

    ca_execute(
            "CREATE OR REPLACE FUNCTION random_upcase_string(l integer) RETURNS varchar AS $$"
            " SELECT array_to_string(ARRAY(SELECT chr((65 + round(random() * 25)) ::integer) FROM generate_series(1, $1)), '') "
            "$$ LANGUAGE sql VOLATILE;"
            );

    ca_execute(
            "CREATE OR REPLACE FUNCTION rnd(l integer) RETURNS integer AS $$"
            " SELECT round(random() * $1) ::integer "
            "$$ LANGUAGE sql VOLATILE;"
            );

    ca_execute(
            "DROP TABLE IF EXISTS events;"
            );

    ca_execute(
            "CREATE TABLE events AS SELECT"
            "  generate_series(1, 10000) :: int8 as id,"
            "  generate_series(1, 12) :: integer as template_info_id,"
            "  rnd(99) ::integer as template_id,"
            "  rnd(15) - 5 ::integer as message_state,"
            "  rnd(65) + 169 ::integer as message_type,"
            "  '+12 1234' || random_numeric_string(4) as mobile_number,"
            "  random_upcase_string(rnd(6) + 6) as alias_name,"
            "  random_upcase_string(rnd(6) + 6) as keyword,"
            "  random_numeric_string(rnd(6) + 6) as short_code,"
            "  (now() - interval '1 month' + rnd(24 * 60 * 30) * interval '1 minute') :: timestamp as gw_out_date,"
            "  0 :: integer as company_id,"
            "  0 :: integer as campaign_id,"
            "  true :: boolean as verified,"
            "  random_upcase_string(2) :: char(2) as state_symbol"
            ";"
            );

    ca_execute(
            "UPDATE events SET "
            "  campaign_id = template_id % 30 + 1,"
            "  company_id = template_id % 3 + 1,"
            "  verified ="
            "  (template_info_id IN(2, 4) AND message_type IN(184, 186, 205))"
            "  OR(template_info_id IN(10) AND message_type IN(183, 205, 206))"
            "  OR(template_info_id IN(1) AND message_type IN(204))"
            "  OR(template_info_id IN(5) AND message_type IN(209))"
            "  OR(template_info_id IN(3, 6, 7, 9, 11, 12, 13) AND message_type IN(184, 205))"
            "  ;"
            );
}

#define BATCH_SIZE 1000;

int main(int argc, char** argv) {
    if (argv[0] == "-h") {
        printf("usage: PGDBNAME=db_name PGUSER=username PGPASSWORD=pass PGHOST=localhost ./skarbnik \n");
    }


    ca_connect();
    generate_test_data();

    // doc = <<DOC
    PGint8 e_id;
    PGint4 e_template_id;
    PGint4 e_template_info_id;
    PGint4 e_message_state;
    PGint4 e_message_type;
    PGvarchar e_mobile_number;
    PGvarchar e_alias_name;
    PGvarchar e_keyword;
    PGvarchar e_short_code;
    PGtimestamp e_gw_out_time;
    PGint4 e_company_id;
    PGint4 e_campaign_id;
    PGbool e_verified;
    PGvarchar e_state_symbol;
    //DOC

    ca_execute("BEGIN work");
    // doc.split("\n").map{|l| l.strip.split(/\s+/)[1][2..-2]}.join(", ")
    ca_execute(
            "DECLARE event_cursor CURSOR FOR SELECT "
            "  id, template_id, template_info_id, message_state, message_type, "
            "  mobile_number, alias_name, keyword, short_code, gw_out_time, "
            "  company_id, campaign_id, verified, state_symbol"
            " FROM events"
            );

    DataPart data_part;
    do {
        data_part = ca_fetch_data_part("FETCH 1000 IN event_cursor");
        int n = data_part->rows;
        for (int i = 0; i < n; i++) {
            int result = PQgetf((PGresult*) data_part->raw,
                    i, /* get field values from tuple 0 */
                    /* type format specifiers (get text by name '#') */
                    //doc.split("\n").map{|l| "%"+l.strip.split(/\s+/)[0][2..-1]}.join(", ")
                    "%int8, %int4, %int4, %int4, %int4, %varchar, %varchar, %varchar, %varchar, %timestamp, %int4, %int4, %bool, %varchar",
                    // i=0; puts doc.split("\n").map{|l| "#{i}, &"+l.strip.split(/\s+/)[1][2..-2]}.join(",\n ")
                    0, &e_id,
                    0, &e_template_id,
                    0, &e_template_info_id,
                    0, &e_message_state,
                    0, &e_message_type,
                    0, &e_mobile_number,
                    0, &e_alias_name,
                    0, &e_keyword,
                    0, &e_short_code,
                    0, &e_gw_out_time,
                    0, &e_company_id,
                    0, &e_campaign_id,
                    0, &e_verified,
                    0, &e_state_symbol
                    ); /* get a point from field num 3 */

        }
    } while (data_part && data_part->rows > 0);

    ca_execute("CLOSE mycursor");
    ca_execute("COMMIT work");


    ca_disconnect();

    //int success;
    //PGint4 i4;
    //PGtext text;
    //PGbytea bytea;
    //PGpoint pt;
    //PGresult *res = PQexec(conn, "SELECT i,t,b,p FROM tbl");

    /* Get some field values from the result (order doesn't matter) */
    //success = PQgetf(res,
    //       0, /* get field values from tuple 0 */
    //     "%int4 #text %bytea %point",
    //   /* type format specifiers (get text by name '#') */
    // 0, &i4, /* get an int4 from field num 0 */
    // "t", &text, /* get a text from field name "t" */
    //2, &bytea, /* get a bytea from field num 2 */
    //3, &pt); /* get a point from field num 3 */


    return (EXIT_SUCCESS);
}

