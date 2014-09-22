<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: tool_asset_sorter.inc,v 1.16 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Asset_Sorter
*
* Purpose
* This tool provides sorting options for assets when displayed in the Asset Map
*
* @author Tom Barrett <tbarrett@squiz.net>
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage core
*/

class Tool_Asset_Sorter extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Asset_Sorter($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paint the interface for this tool
	*
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this tool asset
	*
	* @return void
	* @access public
	* @static
	*/
	public static function paintTool(&$o, $type_code)
	{
		$parent_asset_id = 0;
		$sort_type = 'sort_by_field';
		$sort_field = '';
		$selected_asset_type = '';
		$selected_asset_attribute = '';
		$sort_direction = '';

		if (isset($_POST[$type_code.'_parent']['assetid'])) {
			$parent_asset_id = (int) $_POST[$type_code.'_parent']['assetid'];
		}

		if (isset($_POST[$type_code.'_sort_type'])) {
			$sort_type = addslashes($_POST[$type_code.'_sort_type']);
		}

		if (isset($_POST[$type_code.'_field'])) {
			$sort_field = addslashes($_POST[$type_code.'_field']);
		}

		if (isset($_POST[$type_code.'_asset_type'])) {
			$selected_asset_type = addslashes($_POST[$type_code.'_asset_type']);
		}

		if (isset($_POST[$type_code.'_asset_attribute'])) {
			$selected_asset_attribute = addslashes($_POST[$type_code.'_asset_attribute']);
		}

		if (isset($_POST[$type_code.'_direction'])) {
			$sort_direction = addslashes($_POST[$type_code.'_direction']);
		}

		$sort_by_field = ($sort_type == 'sort_by_field');

		$o->openSection(translate('Sort Options'));

			$o->openField(translate('Parent'));

				asset_finder($type_code.'_parent', $parent_asset_id);
			$o->closeField();
			$o->openField(translate('Sort Type'));

				$options = Array(
						'sort_by_field'		=> 'Asset Field',
						'sort_by_attribute'	=> 'Asset Attribute'
					   );
				combo_box($type_code.'_sort_type', $options, FALSE, $sort_type, 0, 'onchange="setFieldType(this);"');
			$o->closeField();
			$o->openField(translate('Sort Field'), '', '', !$sort_by_field, 'sort_field');

				$options = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
				// unset some things by which we can't or don't want to sort properly
				unset($options['force_secure']);
				unset($options['version']);
				unset($options['charset']);
				unset($options['languages']);
				combo_box($type_code.'_field', $options, FALSE, $sort_field);
			$o->closeField();
			$o->openField(translate('Asset Type'), '', '', $sort_by_field, 'sort_attr_asset');

				hidden_field($type_code.'_asset_type_old', $selected_asset_type);
				asset_type_chooser($type_code.'_asset_type', FALSE, Array($selected_asset_type), TRUE);
			$o->closeField();
			$o->openField(translate('Sort Attribute'), '', '', $sort_by_field, 'sort_attr_attr');

				if ($selected_asset_type == '') {
					echo '<b>['.translate('asset type not selected').']</b>';

				} else {
					$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($selected_asset_type);
					$options = Array();

					foreach ($attrs as $attr_name => $val) {
						$options[$attr_name] = $attr_name;
					}

					asort($options);
					combo_box($type_code.'_asset_attribute', $options, FALSE, $selected_asset_attribute);
				}
			$o->closeField();
			$o->openField(translate('Sort Direction'));

				combo_box($type_code.'_direction', Array('ASC' => 'Ascending', 'DESC' => 'Descending'), FALSE, $sort_direction);
			$o->closeField();
			$o->openField(translate('Note'));

				$o->note(translate('<p>This tool provides sorting functionality for assets in the Asset Map. Assets may be sorted either by \'Asset Field\' or \'Asset Attribute\'.</p><p><b>To sort by \'Asset Field\'</b>, select the \'Asset Field\' sort type and a \'Sort Field\' above.</p><p><b>To sort by \'Asset Attribute\'</b>, select the \'Asset Attribute\' sort type then select an Asset Type above and click \'Commit\'. A list of attributes for the selected asset type will then appear below the Asset Type. Select an attribute and click \'Commit\' to sort by the selected attribute.</p>'));

			$o->closeField();

			$o->addJsInclude(sq_web_path('lib').'/js/tool_asset_sorter.js');

		$o->closeSection();

	}//end paintTool()


	/**
	* Process this tool.  Return false if any errors.
	*
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this asset
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function processTool(&$o, $type_code)
	{
		$parentid = $_REQUEST[$type_code.'_parent']['assetid'];
		if (empty($parentid)) return FALSE;
		$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
		if (is_null($parent)) return FALSE;

		$sort_type = $_POST[$type_code.'_sort_type'];

		$sort_field = $_REQUEST[$type_code.'_field'];
		$sortable_fields = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();

		// Ensure that the user hasn't changed the asset type to avoid asset type + asset attribute mismatches
		$sort_asset_type = $_POST[$type_code.'_asset_type'];
		$old_sort_asset_type = $_POST[$type_code.'_asset_type_old'];
		if (($sort_type == 'sort_by_attribute') && ($sort_asset_type != $old_sort_asset_type)) {
			return FALSE;
		}

		$sort_asset_attribute = '';
		if (($sort_type == 'sort_by_attribute') && empty($sort_asset_type)) {
			return FALSE;
		}

		if (isset($_POST[$type_code.'_asset_attribute'])) {
			$sort_asset_attribute = $_POST[$type_code.'_asset_attribute'];
		}


		$direction = $_REQUEST[$type_code.'_direction'];
		$direction = (strtolower($direction) == 'asc') ? 'ASC' : 'DESC';

		// sanitize parameters for SQL injection attack
		if(!empty($sort_field) && !preg_match('/^[a-z_]+$/i', $sort_field))
			return FALSE;
		if(!empty($sort_asset_type) && !preg_match('/^[a-z_]+$/i', $sort_asset_type))
			return FALSE;


		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Counter to track maximum sort order of assets sorted so far
		$index_delta = 0;

		// There is a limit to CASE statement size in Oracle, that limits it to
		// 127 WHEN-THEN pairs (in theory), so limit to 127 at a time on Oracle
		$chunk_size = (MatrixDAL::getDbType() == 'oci') ? 127 : 500;

		if ($sort_type == 'sort_by_attribute') {
			// Sort the specified asset type only, by Asset Attribute
			$sql = 'SELECT l.linkid
					FROM sq_ast a, sq_ast_attr t, sq_ast_attr_val v, sq_ast_lnk l
					WHERE majorid = :parentid
						AND link_type IN (:link_type_1, :link_type_2)
						AND a.assetid = l.minorid
						AND a.type_code = :sort_asset_type
						AND t.name = :sort_asset_attr
						AND a.assetid = v.assetid
						AND t.attrid = v.attrid ';

			// Special CLOB handling for Oracle
			if (MatrixDAL::getDbType() === 'oci') {
				$sql .= 'ORDER BY TO_CHAR(v.custom_val) '.$direction;
			} else {
				$sql .= 'ORDER BY v.custom_val '.$direction;
			}

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'parentid',        $parentid);
					MatrixDAL::bindValueToPdo($query, 'link_type_1',     SQ_LINK_TYPE_1);
					MatrixDAL::bindValueToPdo($query, 'link_type_2',     SQ_LINK_TYPE_2);
					MatrixDAL::bindValueToPdo($query, 'sort_asset_type', $sort_asset_type);
					MatrixDAL::bindValueToPdo($query, 'sort_asset_attr', $sort_asset_attribute);
					$linkids = MatrixDAL::executePdoAssoc($query, 0);
			} catch (Exception $e) {
				throw new Exception('Unable to get link ids for parent: '.$parentid.' due to database error: '.$e->getMessage());
			}

			// Divide-and-conquer the query, keeping track of how many assets that have been sorted so far (this is important later on)
			Tool_Asset_Sorter::_chunkQuery($linkids, $chunk_size);
			$index_delta = count($linkids);

			// Prepare a query for slotting in the remaining assets (ie; those that are not of the selected asset type)
			$sql = 'SELECT l.linkid
					FROM sq_ast_lnk l
						JOIN sq_ast a ON l.minorid = a.assetid
					WHERE majorid = :parentid
						AND link_type IN (:link_type_1, :link_type_2)
						AND a.type_code != '.MatrixDAL::quote($sort_asset_type).'
					ORDER BY l.sort_order';
		} else {
			// Sort by Asset Field
			$sql = 'SELECT l.linkid
					FROM sq_ast_lnk l
						JOIN sq_ast a ON l.minorid = a.assetid
					WHERE majorid = :parentid
						AND link_type IN (:link_type_1, :link_type_2)
					ORDER BY a.'.$sort_field.' '.$direction.', a.assetid DESC';
		}

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'parentid',    $parentid);
			MatrixDAL::bindValueToPdo($query, 'link_type_1', SQ_LINK_TYPE_1);
			MatrixDAL::bindValueToPdo($query, 'link_type_2', SQ_LINK_TYPE_2);
			$linkids = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get link ids for parent: '.$parentid.' due to database error: '.$e->getMessage());
		}

		// Chunkify SQL queries taking into account the maximum sort order any assets that may have been sorted above.
		// This ensures that the resultant assets are represented with ascending sort order and without duplicated sort orders
		Tool_Asset_Sorter::_chunkQuery($linkids, $chunk_size, $index_delta);

		// Do DB-related magic
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($parent, 'AssetUpdate', Array());

		$o->openSection('Asset Sorting Complete');
			$o->openRaw();
			if ($sort_type == 'sort_by_attribute') {
				$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();
				$asset_type_name = $asset_types[$sort_asset_type]['name'];
				echo $asset_type_name.' child assets of '.get_asset_tag_line($parentid).' have been sorted by '.$sort_asset_attribute;
			} else {
				echo 'The children of '.get_asset_tag_line($parentid).' have been sorted by '.$sortable_fields[$sort_field];
			}
			$o->closeRaw();
		$o->closeSection();
		return TRUE;

	}//end processTool()


	/**
	* Update the asset sort order of the specified asset links.
	* This is processed in chunks to ensure that the SQL "WHEN-THEN" statement pairs arrive safely
	*
	* @param array	$linkids	asset link IDs to update
	* @param int	$chunk_size	number of links to update in one SQL query
	* @param int	$index_delta	the first asset order number to assign
	*
	* @return void
	* @access private
	* @static
	*/
	private static function _chunkQuery($linkids, $chunk_size, $index_delta = 0)
	{
		foreach (array_chunk($linkids, $chunk_size, TRUE) as $chunk) {
			$cases = '';
			foreach ($chunk as $i => $linkid) {
				$cases .= 'WHEN (linkid = '.$linkid.') THEN '.($i+$index_delta).' ';
			}
			$sql = 'UPDATE sq_ast_lnk
					SET sort_order = CASE '.$cases.' ELSE sort_order END
					WHERE linkid IN ('.implode(', ', $chunk).')';
			try {
				$res = MatrixDAL::executeSql($sql);
			} catch (Exception $e) {
				throw new Exception('Unable to update link table with new sort order due to database error: '.$e->getMessage());
			}
		}
	}//end _chunkQuery()


}//end class
?>
