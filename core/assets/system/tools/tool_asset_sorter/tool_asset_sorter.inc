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
* $Id: tool_asset_sorter.inc,v 1.10 2007/12/15 03:48:08 gsherwood Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Asset_Sorter
*
* Purpose
* Provides a tool for making multiple clones of an asset
*
* @author Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.10 $
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
		$o->openSection(translate('tool_asset_sorter_sort_options'));
			$o->openField(translate('parent'));
				asset_finder($type_code.'_parent');
			$o->closeField();
			$o->openField(translate('tool_asset_sorter_sort_field'));
				$options = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
				// unset some things by which we can't or don't want to sort properly
				unset($options['force_secure']);
				unset($options['version']);
				unset($options['charset']);
				unset($options['languages']);
				combo_box($type_code.'_field', $options);
			$o->closeField();
			$o->openField(translate('tool_asset_sorter_sort_direction'));
				combo_box($type_code.'_direction', Array('ASC' => 'Ascending', 'DESC' => 'Descending'));
			$o->closeField();
		$o->closeSection();
		$o->addOnSubmit('
			if (document.getElementById("'.$type_code.'_parent[assetid]").value == 0) {
				alert("'.str_replace('"', '\"', translate('tool_asset_sorter_must_select_parent')).'");
				return false;
			}
		');


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

		$sort_field = $_REQUEST[$type_code.'_field'];
		$sortable_fields = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
		if (!isset($sortable_fields[$sort_field])) return FALSE;

		$direction = $_REQUEST[$type_code.'_direction'];
		$direction = (strtolower($direction) == 'asc') ? 'ASC' : 'DESC';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = MatrixDAL::getDb();

		$sql = 'SELECT l.linkid
				FROM sq_ast_lnk l
					JOIN sq_ast a ON l.minorid = a.assetid
				WHERE majorid = :parentid
					AND link_type IN ('.$db->quote(SQ_LINK_TYPE_1).', '.$db->quote(SQ_LINK_TYPE_2).')
				ORDER BY a.'.$sort_field.' '.$direction.', a.assetid DESC';
		try {
			$query = $db->prepare($sql);
			$query->bindValue(':parentid', $parentid);
			$linkids = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get link ids for parent: '.$parentid.' due to database error: '.$e->getMessage());
		}

		// There is a limit to CASE statement size in Oracle, that limits it to
		// 127 WHEN-THEN pairs (in theory), so limit to 127 at a time on Oracle
		$chunk_size = (MatrixDAL::getDbType() == 'oci') ? 127 : 500;

		foreach (array_chunk($linkids, $chunk_size, TRUE) as $chunk) {
			$cases = '';
			foreach ($chunk as $i => $linkid) {
				$cases .= 'WHEN (linkid = '.$linkid.') THEN '.$i.' ';
			}
			$sql = 'UPDATE sq_ast_lnk
					SET sort_order = CASE '.$cases.' ELSE sort_order END
					WHERE linkid IN ('.implode(', ', $chunk).')';
			try {
				$res = MatrixDAL::executeSqlOne($sql);
			} catch (Exception $e) {
				throw new Exception('Unable to update link table with new sort order due to database error: '.$e->getMessage());
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($parent, 'AssetUpdate', Array());

		$o->openSection('Asset Sorting Complete');
			$o->openRaw();
			echo 'The children of '.get_asset_tag_line($parentid).' have been sorted by '.$sortable_fields[$sort_field];
			$o->closeRaw();
		$o->closeSection();
		return TRUE;

	}//end processTool()


}//end class
?>
