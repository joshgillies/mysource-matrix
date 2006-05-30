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
* $Id: content_type_edit_fns.inc,v 1.6.2.1 2006/05/30 05:19:06 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Content_Type_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Content_Type_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);

	}//end constructor


	/**
	* Paints raw HTML to be printed on the frontend
	*
	* @param object Content_Type	&$asset		the content type we are painting
	* @param string					$prefix		prefix for form elements
	* @param boolean				$editing	are we printing an editing interface
	* @param boolean				$generating	are we genenerating the cache file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $prefix, $editing=false, $generating=false)
	{
		$output = $asset->attr('html');
		if ($generating) {
			// escape dangerous content, eg. <?php, <?
			$output = str_replace('<?', "<?php echo '<'.'?'; ?>", $output);
		}
		echo $output;

	}//end paint()


	/**
	* Prints any icons the content type needs next to the cell properties
	*
	* @param object Content_Type	&$asset	the content type object
	* @param string					$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendIcons(&$asset, $prefix)
	{

	}//end paintBackendIcons()


}//end class

?>
