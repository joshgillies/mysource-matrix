TRUNCATE sq_asset_type;
TRUNCATE sq_asset_attribute;
TRUNCATE sq_asset_attribute_default;
#TRUNCATE sq_asset_type;
#TRUNCATE sq_asset_type;



INSERT INTO sq_asset_type (type_code, level, parent_type) VALUES ('site', 2, 'page');
INSERT INTO sq_asset_type_inherited (inherited_type_code, type_code) VALUES ('page', 'site');

INSERT INTO sq_asset_type (type_code, level, parent_type) VALUES ('site_meta', 3, 'site');
INSERT INTO sq_asset_type_inherited (inherited_type_code, type_code) VALUES ('page', 'site_meta');
INSERT INTO sq_asset_type_inherited (inherited_type_code, type_code) VALUES ('site', 'site_meta');



INSERT INTO sq_asset_attribute (type_code, type, name) VALUES ('page', 'text', 'short_name');
INSERT INTO sq_asset_attribute (type_code, type, name) VALUES ('site_meta', 'text', 'url');








INSERT INTO sq_asset_type (type_code, level) 
    VALUES ('page', 1);
INSERT INTO sq_asset_attribute (attributeid, type_code, type, name) 
    VALUES (1, 'page', 'text', 'short_name');
INSERT INTO sq_asset_attribute (attributeid, type_code, type, name) 
    VALUES (2, 'page', 'text', 'show_in_menu');

INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) 
    VALUES (1, 'page', 'Pages Short Name');
INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) 
    VALUES (2, 'page', '1');


INSERT INTO sq_asset_type (type_code, level) 
    VALUES ('site', 2);
INSERT INTO sq_asset_attribute (attributeid, type_code, type, name) 
    VALUES (3, 'site', 'text', 'description');
INSERT INTO sq_asset_attribute (attributeid, type_code, type, name) 
    VALUES (4, 'site', 'text', 'design');

INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) 
    VALUES (1, 'site', 'Site\'s Short Name');
INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) 
    VALUES (3, 'site', 'consectetuer adipiscing elit, sed diam nonummy');


INSERT INTO sq_asset (assetid, type_code, name) VALUES (1, 'site', 'Test Site');
INSERT INTO sq_asset_attribute_value (assetid, attributeid, value) VALUES (1, 1, 'Test Site');
INSERT INTO sq_asset_attribute_value (assetid, attributeid, value) VALUES (1, 3, 'Lorem blah blah');
#INSERT INTO sq_asset_attribute_value (assetid, attributeid, value) VALUES (1, 1, 'Record Page');
INSERT INTO sq_asset (assetid, type_code, name) VALUES (2, 'page', 'Test Page');



SELECT a.attributeid, a.name, v.value
FROM sq_asset_attribute a, sq_asset_attribute_value v
WHERE a.attributeid = v.attributeid
  AND v.assetid = 1
;



/* GET ALL THE DEFAULT VALUES */

SELECT a.attributeid, a.name, d.value
FROM sq_asset_attribute a, sq_asset_attribute_default d, sq_asset_type t
WHERE a.attributeid = d.attributeid
  AND d.type_code= t.type_code
  AND d.type_code IN ('page', 'site')
ORDER BY a.attributeid, t.level
;

/* SUB QUERY VERSION */

/*
SELECT a.attributeid, a.name, d.value
FROM sq_asset_attribute a, sq_asset_attribute_default d, sq_asset_type t
WHERE a.attributeid = d.attributeid
  AND d.type_code IN ('page', 'page_house')
  AND t.type_code = d.type_code
  AND t.level = (SELECT MAX(t.level)
                  FROM sq_asset_attribute a, sq_asset_attribute_default d, sq_asset_type t
                  WHERE a.attributeid = d.attributeid
                    AND d.type_code IN ('page', 'page_house')
                    AND t.type_code = d.type_code
                  GROUP BY a.attributeid);
*/






