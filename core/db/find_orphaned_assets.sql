
# Structure
# asset(assetid, name, ...);
# asset_link(parentid, parentorder, childid, childorder)


SELECT a.assetid
FROM sq_asset a LEFT OUTER JOIN asset_link l ON a.assetid = l.parentid
WHERE l.parentid IS NULL

