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
* $Id: design_css.inc,v 1.3 2004/08/30 03:50:31 amiller Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design.inc';

/**
* Design_Css
*
* Purpose
* This is a CSS-specific asset which is parsed on upload/edit and can be linked
* to an existing design. This asset supports all the design_areas that can be used
* in standard Designs
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css extends Design
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Design_Css($assetid=0)
	{
		$this->Design($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$new_name = strtolower(trim($this->attr('id_name')));
		
		// no ampersands in css file name
		$new_name = str_replace('&', '_and_',  $new_name);
		
		// no spaces in css file name
		$new_name = preg_replace('/\\s+/', '_', $new_name);
		
		// taken (in part) from info here -> http://www.w3.org/Addressing/URL/5_URI_BNF.html
		$new_name = preg_replace('/[^a-zA-Z0-9\-$_@.!*~(),]/', '',  $new_name);
		
		// ensure a virtual .css file extension
		if (substr($new_name, -4) != '.css') $new_name = $new_name.'.css';
		
		$this->setAttrValue('id_name', $new_name);
		
		return parent::create($link);

	}//end create()
	
	
	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array('design_css_customisation' => Array('card' => 'M', 'exclusive' => false),
											 'file'                 	=> Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_3 => Array('design_area'          	=> Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$minor = &$am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// special processing if we are deleting a TYPE_2 linked file (a file overridding a design file)
		if (is_a($minor, 'file')) {

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_css_customisation', true, 'major', 'customisation');
			foreach($customisation_links as $link) {
				$customisation = &$am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_3, 'file', false);

				foreach ($file_links as $link) {
					if ($link['minorid'] == $delete_link['minorid']) {
						// found the link to delete
						if (!$customisation->deleteLink($link['linkid'])) {
							trigger_error('Failed deleting file link from customisation', E_USER_WARNING);
							$am->forgetAsset($customisation);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return false;
						}
					}
				}// end foreach $file_links

				$am->forgetAsset($customisation);

			}//end foreach customisation
		} //end is_a file

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deleteLink()


	/**
	* Does the actual removing of the link for the customisation
	*
	* The default deleteLink() replaces the (Type-2) links of any customised design files with a Type-3
	* to the uncustomised version. This screws things around when creating the customised versions in
	* the first place.
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteExistingLink($linkid) {
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$minor = &$am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end deleteExistingLink()
	
	
	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$this->paint($this);
	}//end printFrontend()


	/**
	* Returns the parent design/design_customisation that this customisation is customising
	*
	* @return mixed object Design or null
	* @access public
	*/
	function &getParentDesign()
	{
		// a css file is its own customisation
		return $this;

	}//end getParentDesign()


	/**
	* Outputs the either as straight HTML or PHP code for the var, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param Array	$args	array of any extra arguments that existed in the print command
	*
	* @return void
	* @access public
	*/
	function printVar($var, $args)
	{
		switch($var) {
			case 'css_header' :
echo '/*   <?php echo SQ_CONF_SYSTEM_NAME."\n"; ?>
	  Content (c) Copyright <?php echo SQ_CONF_SYSTEM_OWNER."\n"; ?>
	Running <?php echo SQ_SYSTEM_LONG_NAME; ?> - <?php echo SQ_SYSTEM_URL."\n"; ?>
   Developed by Squiz  - http://www.squiz.net
  (c) Copyright <?php echo date(\'Y\') ?>. All rights reserved.
  Page generated: <?php echo date(\'d F Y\')."\n"; ?>
*/

';
			break;
			
			default :
				parent::printVar($var, $args);

		}// end switch

	}//end printVar()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		$this->_tmp['generate_design'] = false;
		return parent::processBackend($o, $link);

	}// end processBackend()


	/**
	* Returns the link information for the design area with the given id_name or all design area links
	*
	* @param string	$id_name	the id of the design area to get
	*							if blank returns links for all design areas
	*
	* @return mixed Array or null
	* @access public
	*/
	function getDesignAreaLink($id_name='')
	{
		if ($id_name) {
			return @$GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, 'design_area', false, $id_name);
		} else {
			return @$GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'design_area', false);
		}

	}//end getDesignAreaLink()
	

	/**
	* Paints out our design
	*
	* @param object Asset	$ASSET		the asset that we are going to print the body for
	* @param array			$FILE_URLS	an array of filename => URL to use to print files in the design
	*									doesnt need to be passed in unless some URLs are overridden by
	*									a customisation of the design
	*
	* @return void
	* @access public
	*/
	function paint(&$ASSET, $FILE_URLS=Array())
	{
		// generate an array of all the URLs we are going to need for the files in this design
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', false);

		foreach ($file_links as $link) $file_link_ids[] = $link['minorid'];

		$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_unique($file_link_ids));
		$file_link_urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($file_link_ids));

		$system_root_url = sq_web_path('root_url');
		foreach ($file_link_info as $minorid => $file_info) {
			$file_url = $file_link_urls[$minorid];
			if ($file_url == '') $file_url = $system_root_url.'/?a='.$minorid;
			$FILE_URLS[$file_info['name']] = $file_url;
		}

		$DATA_PATH = sq_web_path('data'); // this works because we know that all design areas are not going to be restricted
		$LIB_PATH  = sq_web_path('lib');

		// work out where the design file is
		if (SQ_ROLLBACK_VIEW) {
			// get an older version of our design file
			$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $this->data_path_suffix.'/design_file.php';
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = $fv->_checkOutCheck($rep_file, null, $then);
			$file_path = $info['source_file'];
		} else {
			$file_path = $this->data_path.'/design_file.php';
		}

		// Now we just include the design file to get it to print
		require_once $file_path;

	}//end paint()

}//end class

?>
