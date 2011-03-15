<?php

$root_dir = dirname(dirname(dirname(dirname(__FILE__))));

require_once $root_dir.'/core/include/init.inc';

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$dbType = MatrixDAL::getDbType();
if ($dbType == 'pgsql') {
	// Postgres.
	// Remove old indexes and keys.
	MatrixDAL::executeSql('ALTER TABLE ONLY sq_suite_product DROP CONSTRAINT suite_product_pk');
	MatrixDAL::executeSql('DROP INDEX sq_suite_product_type');

	// Create a new sequence.
	// SELECT FROM information_schema.sequences won't work with postgres 8.1, so this is the only way to check
	try {
	$sequence = MatrixDAL::executeSqlAssoc("SELECT * from sq_suite_seq");
	} catch (Exception $e) {
		MatrixDAL::executeSql('CREATE SEQUENCE sq_suite_seq INCREMENT BY 1');
	}


	// Add new columns.
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ADD COLUMN suiteid INTEGER');
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ADD COLUMN url VARCHAR(2000)');
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ADD COLUMN token VARCHAR(30)');

	// Remove unused column.
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product DROP COLUMN knows_me_as');

	// Populate data into not null columns.
	MatrixDAL::executeSql("UPDATE sq_suite_product SET suiteid=nextval('sq_suite_seq')");
	MatrixDAL::executeSql("UPDATE sq_suite_product SET url=''");
	$products = MatrixDAL::executeSqlAssoc('SELECT suiteid, connection FROM sq_suite_product');
	foreach ($products as $product) {
		$suiteid    = array_get_index($product, 'suiteid', NULL);
		$connection = array_get_index($product, 'connection', NULL);
		if ($suiteid === NULL || $connection === NULL) {
			continue;
		}

		$connection = @unserialize($connection);
		if ($connection === FALSE) {
			continue;
		}

		$url = array_get_index($connection, 'url', NULL);
		if ($url === NULL) {
			continue;
		}

		unset($connection['url']);
		$query = MatrixDAL::preparePdoQuery('UPDATE sq_suite_product SET url=:url, connection=:connection WHERE suiteid=:id');
		MatrixDAL::bindValueToPdo($query, 'url', $url);
		MatrixDAL::bindValueToPdo($query, 'connection', serialize($connection));
		MatrixDAL::bindValueToPdo($query, 'id', $suiteid);
		MatrixDAL::execPdoQuery($query);
	}//end foreach
	
	// Set the not null constraint on columns.
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ALTER COLUMN suiteid SET NOT NULL');
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ALTER COLUMN url SET NOT NULL');

	// Set the new constraints and keys
	MatrixDAL::executeSql('ALTER TABLE ONLY sq_suite_product ADD CONSTRAINT suite_product_pk PRIMARY KEY (suiteid)');
	MatrixDAL::executeSql('CREATE INDEX sq_suite_product_type ON sq_suite_product (systemid, type, status)');
} else {
	// Oracle.
	// Remove old indexes and keys.
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product DROP CONSTRAINT suite_product_pk');
	MatrixDAL::executeSql('DROP INDEX sq_suite_product_type');

	// Create a new sequence.
	$sequence = MatrixDAL::executeSqlAssoc("SELECT sequence_name FROM user_sequences WHERE sequence_name='SQ_SUITE_SEQ'");
	if (empty($sequence)) {
		MatrixDAL::executeSql('CREATE SEQUENCE sq_suite_seq INCREMENT BY 1');
	}

	// Add new columns.
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ADD suiteid INTEGER');
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ADD url VARCHAR2(2000)');
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ADD token VARCHAR2(30)');

	// Remove unused column.
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product DROP COLUMN knows_me_as');

	// Populate data into not null columns.
	MatrixDAL::executeSql("UPDATE sq_suite_product SET suiteid=sq_suite_seq.nextVal");
	MatrixDAL::executeSql("UPDATE sq_suite_product SET url=''");
	$products = MatrixDAL::executeSqlAssoc('SELECT suiteid, connection FROM sq_suite_product');
	foreach ($products as $product) {
		$suiteid    = array_get_index($product, 'suiteid', NULL);
		$connection = array_get_index($product, 'connection', NULL);
		if ($suiteid === NULL || $connection === NULL) {
			continue;
		}

		$connection = @unserialize($connection);
		if ($connection === FALSE) {
			continue;
		}

		$url = array_get_index($connection, 'url', NULL);
		if ($url === NULL) {
			continue;
		}

		unset($connection['url']);
		$query = MatrixDAL::preparePdoQuery('UPDATE sq_suite_product SET url=:url, connection=:connection WHERE suiteid=:id');
		MatrixDAL::bindValueToPdo($query, 'url', $url);
		MatrixDAL::bindValueToPdo($query, 'connection', serialize($connection));
		MatrixDAL::bindValueToPdo($query, 'id', $suiteid);
		MatrixDAL::execPdoQuery($query);
	}//end foreach
	
	// Set the not null constraint on columns.
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product MODIFY suiteid NOT NULL');
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product MODIFY url NOT NULL');

	// Set the new constraints and keys
	MatrixDAL::executeSql('ALTER TABLE sq_suite_product ADD CONSTRAINT suite_product_pk PRIMARY KEY (suiteid)');
	MatrixDAL::executeSql('CREATE INDEX sq_suite_product_type ON sq_suite_product (systemid, type, status)');
}//end if
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

?>
