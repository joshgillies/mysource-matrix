<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*
*/


		/**
		* THIS SCRIPT WILL GENERATE FOR YOU THE SQL QUERIES NEEDED TO CHANGE THE METADATA SORTING OPTION
		* THIS SCRIPT WILL NOT AFFECT THE SYSTEM
		*/
		require '../core/include/init.inc';

		$verbose = FALSE;

        $root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
        $GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
        $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);


		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$db = NULL;
		// PHP 5
		if (version_compare(phpversion(), 5) >= 0) {
			$db = MatrixDAL::getDb();
		} else {
			$db =& $GLOBALS['SQ_SYSTEM']->db;
		}


		// get the attribute id
		$select_attribute = 'select attrid from sq_ast_attr where type_code = \'page_asset_listing\' and name = \'metadata_sort_type\'';

		$attribute_id = NULL;

		// PHP 5
		if (version_compare(phpversion(), 5) >= 0) {
        	$query = $db->prepare($select_attribute);
	        try {
    	        $attribute_id = MatrixDAL::executePdoOne($query);
	        } catch (Exception $e) {
    	        throw new Exception($e.getMessage());
	        }//end
		} else {
			$attribute_id = $db->getOne($select_attribute);
    	    assert_valid_db_result($attribute_id);
		}

		// if the attribute is not found exit
		if (empty($attribute_id)) {
			echo 'The attribute "metadata_sort_type" for "page_asset_listing" does not exists';
			exit();
		}


		// get all the page_asset_listings
		$assets_id =  $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_asset_listing', TRUE);
        // if no asset listings are found exit
        if (empty($assets_id)) {
            echo 'NO PAGE ASSET LISTINGS FOUND'."\n";
            exit();
        }
        if ($verbose) {
			bam('All the page_asset_listing ids');
			bam($assets_id);
		}

		// USER MANUAL
		echo '*****************************************'."\n";
		echo '* THE FOLLOWING ASSETS WILL BE AFFECTED *'."\n";
		echo '*****************************************'."\n";
		bam($assets_id);

        // create the SQL IN statement
        $asset_listing_ids = '';
		$_tmp_assets_id = Array();
        if (count($assets_id) > 1) {
            foreach ($assets_id as $key => $value) {
				$_tmp_assets_id = array_merge($_tmp_assets_id, Array($key => $db->quote((string)$value)));
            }
            $asset_listing_ids = implode(',', $_tmp_assets_id);
        } else {
            $asset_listing_ids = "'".$assets_id[0]."'";
        }

		// get all the page_asset_listings with a custom attribute
		$select = "select atv.assetid from sq_ast_attr at, sq_ast_attr_val atv where at.name like 'metadata_sort_type' and at.attrid = atv.attrid and atv.assetid in (".$asset_listing_ids.");";
		$with_custom_attr_assets_id = Array();
        // PHP 5
        if (version_compare(phpversion(), 5) >= 0) {
			$result = NULL;
			$query = $db->prepare($select);
			try {
				$result = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception('Could not get the '.$e->getMessage());
			}//end
			foreach ($result as $value) {
				$with_custom_attr_assets_id = array_merge($with_custom_attr_assets_id, Array($value['assetid']));
			}bam($result);
		} else {
	        $result = $db->query($select);
    	    assert_valid_db_result($result);
        	while (DB_OK === $result->fetchInto($row)) {
            	$with_custom_attr_assets_id = array_merge($with_custom_attr_assets_id, Array($row['assetid']));
	        }
	        $result->free();
		}

        if ($verbose) {
		    bam('page_asset_listing ids of assets with custom attribute');
	        bam($with_custom_attr_assets_id);
		}


		// find the assets that we need to create a custom attribute
		$create_custom_attr_id = array_diff($assets_id, $with_custom_attr_assets_id);
        if ($verbose) {
			bam('Assets that need to have a custom value');
			bam($create_custom_attr_id);
		}
		// USER MANUAL
		echo "\n\n";
		echo "***************************************************************\n";
		echo "* WARNING: DOUBLE CHECK THE SQL QUERIES BEFORE EXECUTING THEM *\n";
		echo "***************************************************************\n\n";



		// USER MANUAL
		echo "\n\n";
		echo '*****************************************************************************'."\n";
		echo '* TO SET THE METADATA SORTING OPTION TO "RAW" RUN THE FOLLOWING SQL QUERIES *'."\n";
        echo '*****************************************************************************'."\n\n";

		// create a custom value for the assets
		foreach($create_custom_attr_id as $id) {
			echo 'INSERT INTO sq_ast_attr_val(assetid, attrid, custom_val) values (\''.$id.'\', \''.$attribute_id.'\', \'raw\');'."\n";
		}


        // create the SQL IN statement
        $asset_listing_ids = '';
        if (count($assets_id) > 1) {
            foreach ($assets_id as $key => $value) {
                $assets_id[$key] = $db->quote((string)$value);
            }
            $asset_listing_ids = implode(',', $assets_id);
        } else {
            $asset_listing_ids = $assets_id[0];
        }

		// get all the assets that have a custom value for the attribute

		$where = '';
		foreach ($assets_id as $id) {
            if (!empty($where)) {
                // we are NOT the first element
                $where .= ' OR ';
            }
            $where .= '( assetid = '.$id.' AND attrid = \''.$attribute_id.'\')';
		}

		$update = 'UPDATE sq_ast_attr_val set custom_val = \'raw\' where '.$where.';';
		echo $update."\n";

        // USER MANUAL
        echo "\n\n";
        echo '**************************************************************************************'."\n";
        echo '* TO SET THE METADATA SORTING OPTION TO "PRESENTATION" RUN THE FOLLOWING SQL QUERIES *'."\n";
        echo '**************************************************************************************'."\n\n";

        // create a custom value for the assets
        foreach($create_custom_attr_id as $id) {
            echo 'INSERT INTO sq_ast_attr_val(assetid, attrid, custom_val) values (\''.$id.'\', \''.$attribute_id.'\', \'presentation\');'."\n";
        }
        $update = 'UPDATE sq_ast_attr_val set custom_val = \'presentation\' where '.$where.';';
        echo $update."\n";


        $GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
