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
* $Id: tool_asset_sorter.inc,v 1.3.2.1 2007/02/02 03:15:16 skim Exp $
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
* @version $Revision: 1.3.2.1 $
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
	function paintTool(&$o, $type_code)
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
	function processTool(&$o, $type_code)
	{
		$parentid = $_REQUEST[$type_code.'_parent']['assetid'];
		if (empty($parentid)) return FALSE;
		$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
		if (is_null($parent)) return FALSE;

		$sort_field = $_REQUEST[$type_code.'_field'];
		$sortable_fields = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
		if (!isset($sortable_fields[$sort_field])) return FALSE;

		$direction = $_REQUEST[$type_code.'_direction'];
		$direction = (strtolower($direction) == 'asc') ? 'ASC' : 'DESC';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT l.linkid
				FROM sq_ast_lnk l
					JOIN sq_ast a ON l.minorid = a.assetid
				WHERE majorid = '.$db->quote($parentid).'
					AND link_type IN ('.$db->quote(SQ_LINK_TYPE_1).', '.$db->quote(SQ_LINK_TYPE_2).')
				ORDER BY a.'.$sort_field.' '.$direction;
		$linkids =  $db->getCol($sql);
		assert_valid_db_result($linkids);

		foreach (array_chunk($linkids, 500, TRUE) as $chunk) {
			$cases = '';
			foreach ($chunk as $i => $linkid) {
				$cases .= 'WHEN (linkid = '.$linkid.') THEN '.$i.' ';
			}
			$sql = 'UPDATE sq_ast_lnk
					SET sort_order = CASE '.$cases.' ELSE sort_order END
					WHERE linkid IN ('.implode(', ', $chunk).')';
			$res = $db->query($sql);
			assert_valid_db_result($res);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
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