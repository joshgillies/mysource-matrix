<?php

/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: search_page_edit_fns.inc,v 1.5 2003/11/18 15:45:00 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Search_Page_Edit_Fns
*
* Edit functions for search page.
*
* @author  Dominic WOng <dwong@squiz.net>
* @version $Revision: 1.5 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/

require_once SQ_PACKAGES_PATH.'/cms/page/page_edit_fns.inc';

class Search_Page_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Search_Page_Edit_fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor

	/**
	* Paints the interface for choosing the root asset.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	* 
	*/
	function paintRootAsset(&$asset, &$o, $prefix) 
	{
		$name = $prefix."_root_asset";
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$assetid = $asset->attr('root_assetid');
		if ($asset->writeAccess('attributes')) {
			asset_finder($name, $assetid);
		} else {
			if ($assetid > 0) {
				$rootAsset =& $am->getAsset($assetid);
				echo "<a href=\"".$rootAsset->getBackendHref()."\">".$rootAsset->name."</a>";
			} else {
				echo "No root asset specified.";
			}
		}

	} // end paintRootAsset()

	/**
	* Processes the interface for choosing the root asset.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* 
	*/
	function processRootAsset(&$asset, &$o, $prefix) 
	{
		$name = $prefix."_root_asset";
		$assetid = null;

		if (isset($_REQUEST[$name])) {
			$assetid = $_REQUEST[$name];
		}
		return $asset->setAttrValue('root_assetid', $assetid);
	} // end processRootAsset()


	/**
	* Paints the interface for restricting the asset types returned.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	* 
	*/
	function paintAssetTypeRestriction(&$asset, &$o, $prefix)
	{
		?>Implement me!<?
	} // end paintAssetTypeRestriction()

	/**
	* Processes the interface for restricting the asset types returned.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* 
	*/
	function processAssetTypeRestriction(&$asset, &$o, $prefix)
	{

	} // end processAssetTypeRestriction()

	/**
	* Paints the interface for adding and configuring search fields.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	* 
	*/
	function paintFields(&$asset, &$o, $prefix)
	{
		$fields = $asset->attr('fields');
		if (is_scalar($fields))
			$fields = unserialize($fields);

		$queryTypes = Array(
			'include_all' => 'Include All', 
			'exclude' => 'Exclude', 
			'metadata' => 'Metadata', 
		); 

		$i = 0;
		?>
		<table class="sq-backend-table">
		<tr>
		<td class="sq-backend-table-header">Name</td>
		<td class="sq-backend-table-header">Type</td>
		<td class="sq-backend-table-header">Parameters</td>
		<?php 

		if ($asset->writeAccess('attributes')) {
			echo "<td class=\"sq-backend-table-header\">Delete?</td>";
		}
		echo "</tr>";
		foreach (array_values($fields) as $fieldName => $fieldData) {
			echo "<tr>";
			if ($asset->writeAccess('attributes')) {
				?>
				<tr>
				<td class="sq-backend-data"><?php text_box($prefix.'_fields[' .$i. '][name]', $fieldData['name'])?></td>
				<td class="sq-backend-data"><?php combo_box($prefix.'_fields[' .$i .'][type]', $queryTypes, false, $fieldData['type'])?></td>
				<td class="sq-backend-data"><?php $this->paintFieldParameters($asset, $prefix."_fields[$i]", $fieldData)?></td>
				<td class="sq-backend-data"><?php check_box($prefix.'_deleted_fields['. $i. ']') ?></td>
				</tr>
				<?php
			} else {
				?>
				<tr>
				<td class="sq-backend-data"><?php echo $fieldData['name'] ?></td>
				<td class="sq-backend-data"><?php echo $queryTypes[$fieldData['type']] ?></td>
				<td class="sq-backend-data"><?php $this->paintFieldParameters($asset, $prefix."_fields[$i]", $fieldData)?></td>
				</tr>
				<?php
			}
			echo "</li>";
			$i++;
		}
		echo "</table>";
		if ($asset->writeAccess('attributes')) {
			$i++;
			?>Add field (name): <?php
			text_box($prefix."_fields[$i][name]", '', 15);
			?><br>
			Query type:
			<?php
			echo combo_box($prefix.'_fields['. $i .'][type]', $queryTypes, false, array());
		}
		return true;
	} // end paintFields()

	/**
	* Processes the interface for adding and configuring search fields.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* 
	*/
	function processFields(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			return false;
		}
		$changed = false;
		$fields = $asset->attr('fields');

		if (is_scalar($fields))
			$fields = unserialize($fields);

		$fields_post =& $_POST[$prefix.'_fields'];
		$deleted_fields =& $_POST[$prefix.'_deleted_fields'];

		$fields = Array();
		//return $asset->setAttrValue('fields', $fields);

		if(isset($fields_post) && count($fields_post) > 0) {
			foreach ($fields_post as $i => $fieldData) {
				if (isset($deleted_fields[$i]))
					continue;
				if($fieldData['name'] == '')
					continue;
				if (isset($fields[$fieldData['name']]))
					continue;

				$fields[$fieldData['name']] = $fieldData;
			}
		}

		return $asset->setAttrValue('fields', $fields);
	} // end processFields()

	/**
	* Paints the interface for the fields' parameters.
	*
	* @param object Search_Page			&$asset			the search page asset
	* @param string						$fieldPrefix	The field prefix.
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* 
	*/
	function paintFieldParameters(&$asset, $fieldPrefix, $fieldData)
	{
		switch ($fieldData['type']) {
			case 'metadata':
				$metadata_field =& $fieldData['params']['metadata_field'];
				if (!isset($metadata_field)) {
					$metadata_field = '0';
				}
				if ($asset->writeAccess('attributes')) {
					asset_finder($fieldPrefix. '[params][metadata_field]', $metadata_field, Array('metadata_field' => 'D'));
				} else {
					if ($metadata_field > 0) {
						$fieldAsset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field);
						echo $fieldAsset->name . ' #'. $fieldAsset->id;
					} else {
						echo 'No metadata field specified.';
					}
				}
			break;

			case 'include_all':
			case 'exclude':
			default:
			break;
		}
	} // end paintFieldParameters()



}//end class

?>