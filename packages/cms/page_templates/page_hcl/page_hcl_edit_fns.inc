<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: page_hcl_edit_fns.inc,v 1.1 2004/01/13 05:53:59 diarandine Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';

/**
* Page_HCL_Edit_Fns
*
* Purpose
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_HCL_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*/
	function Page_HCL_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor

	
	/** 
	* Paints the root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param String						$prefix		prefix for the html doc element name
	*						
	* @return boolean
	* @access private
	*/
	function paintRootNode(&$asset, &$o, $prefix) {

		if (!$asset->writeAccess('links')) return false; // if details page is not locked, do not proceed

		$rootid	= $asset->attr('rootid'); // get the attr rootid if available

		if (empty($rootid)) {
				asset_finder($prefix.'_rootid'); // if $rootid is null, then just display select box
			} else {
				asset_finder($prefix.'_rootid', $rootid); // otherwise display which root node is selected
		}
		
		return true;

	}//end paintRootNode()


	/** 
	* Processes the value input from root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param String						$prefix		prefix for the html doc element name
	*						
	* @return void
	* @access private
	*/
	function processRootNode(&$asset, &$o, $prefix) {

		if (isset($_POST[$prefix.'_rootid']['assetid'])) {
			$asset->setAttrValue('rootid', $_POST[$prefix.'_rootid']['assetid']);
		} else {
			echo "<B>WARNING!</B> No rootid was selected, HCL template will not work properly.";
		}

		$asset->saveAttributes();

	}//end processRootNode()


	/** 
	* Paints the list of stored metadata schemas for HCL template
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param String						$prefix		prefix for the html doc element name
	*						
	* @return boolean
	* @access private
	*/
	function paintHCLSchemas(&$asset, &$o, $prefix) {

		$am = &$GLOBALS['SQ_SYSTEM']->am; // instantiating Asset Manager
		if (!$asset->writeAccess('links')) return false; // if details page is not locked, do not proceed

		// 1). Creating the list of schemas stored in attributes Array of this page

		$schemas = $asset->attr('schemas'); // get the attr array of stored selection

		?>
			<table border="1" cellpadding="2" cellspacing="0" class="sq-backend-table">
		<?
			
			if (isset($schemas)) {

				echo "<tr><td class=\"sq-backend-table-header\"><b>Schema name</b></td><td width=50 class=\"sq-backend-table-header\"><b>Delete ?</b></td></tr>\n";

				foreach ($schemas as $schema_id) {
				
					$schema_asset = &$am->getAsset($schema_id); 
					if (is_null($schema_asset)) continue;
					$schema_name = $schema_asset->attr('name');
					
					echo "<tr><td class=\"sq-backend-table-cell\">".$schema_name."</td><td class=\"sq-backend-table-cell\">\n";
						check_box($prefix.'_schemas[]', $schema_id); 
				    echo "</td></tr>\n";

				} // end foreach schemas

			} else {
				echo "<tr><td colspan=\"2\" class=\"sq-backend-table-cell\">There are currently no Metadata schemas in this list</td></tr>\n";
			}

			echo "<tr><td colspan=\"2\" class=\"sq-backend-table-cell\">Add schema: "; 
				asset_finder($prefix.'_newschema', 0, Array('metadata_schema' => 'I')); 
			echo "</td></tr></table>\n";

		return true;

	}//end paintHCLSchemas()


	/** 
	* Process adding of the new schema to list, or schema deletion
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param String						$prefix		prefix for the html doc element name
	*						
	* @return void
	* @access private
	*/
	function processHCLSchemas(&$asset, &$o, $prefix) {

		if (!isset($_POST[$prefix.'_newschema']['assetid'])) return false;
		$new_schema  = $_POST[$prefix.'_newschema']['assetid']; // submitted root id

		$stored_schemas = $asset->attr('schemas'); // schemas that are already in the system

		// now checking if there were any checkboxes marked->delete those schemas from list
		if (isset($_POST[$prefix.'_schemas'])) {
			$marked_schemas  = $_POST[$prefix.'_schemas'];
		}

		// if there were some schemas marked for deletion...
		if (isset($marked_schemas)) {
			foreach ($marked_schemas as $delete_id) {
				unset($stored_schemas[$delete_id]); // removing element(s) from schemas array		
			}
		}

		// now finished deleting, let's add new value to our list, but first check if value exists
		if ($new_schema && !array_key_exists($new_schema, array_keys($stored_schemas))) {
			$stored_schemas[$new_schema] = $new_schema;
		}
		
		// assigning the new value
		$asset->setAttrValue('schemas', $stored_schemas);

	}//end processHCLSchemas()
	

}//end class

?>