// require('../../config.js');
// const pgp = require('pg-promise')();

// const db = pgp(process.env.DB_URL);

// const schemaSql = `
//     -- Extensions
//     CREATE EXTENSION IF NOT EXISTS pg_trgm;

//     -- Drop (droppable only when no dependency)
//     DROP INDEX IF EXISTS todos_idx_text;
//     DROP INDEX IF EXISTS todos_idx_ts;
//     DROP TABLE IF EXISTS todos;

//     CREATE TABLE todos (
//         id              serial PRIMARY KEY NOT NULL,
//         mood            mood NOT NULL,
//         text            text NOT NULL,
//         ts              bigint NOT NULL DEFAULT (extract(epoch from now())),
//         "doneTs"          integer NOT NULL DEFAULT 0
//     );
//     CREATE INDEX todos_idx_ts ON todos USING btree(ts);
//     CREATE INDEX todos_idx_text ON todos USING gin(text gin_trgm_ops);
// `;

// const dataSql = `
//     -- Populate dummy todos
//     INSERT INTO todos (mood, text, ts)
//     SELECT
//         'Clear',
//         'word' || i || ' word' || (i+1) || ' word' || (i+2),
//         round(extract(epoch from now()) + (i - 1000000) * 3600.0)
//     FROM generate_series(1, 1000000) AS s(i);
// `;

// db.none(schemaSql).then(() => {
//     console.log('Schema created');
//     db.none(dataSql).then(() => {
//         console.log('Data populated');
//         pgp.end();
//     });
// }).catch(err => {
//     console.log('Error creating schema', err);
// });
