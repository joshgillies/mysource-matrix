<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design.inc,v 1.49 2003/09/29 23:04:23 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design extends Design_Area
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Design($assetid=0)
	{
		$this->Design_Area($assetid);

	}//end constructor


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
					 SQ_LINK_TYPE_2 => Array('design_customisation' => Array('card' => 'M', 'exclusive' => false),
											 'file'                 => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_3 => Array('design_area'          => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// set an initial web path
		$initial_path = strtolower($this->name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		if (!$this->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
	}//end printFrontend()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string $sort_order	the position in the links list that this link should take, 
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	* @see Asset::moveLinkPos()
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_error('Links under Designs are system controlled and therefore cannot be moved', E_USER_WARNING);
		return false;

	}//end moveLinkPos()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff 
	* to occur but not cause integrity issues during script execution
	* returns true it it's successfull
	*
	* @return boolean
	* @access private
	*/
	function _linksUpdated()
	{
		if (!parent::_linksUpdated()) return false;
		if (isset($this->_tmp['design_areas'])) {
			foreach(array_keys($this->_tmp['design_areas']) as $id_name) unset($this->_tmp['design_areas'][$id_name]);
		}
		return true;

	}//end _linksUpdated()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param string			$value		the value that is to be associated with this link
	* @param string			$sort_order	the position in the links list that this link should take,
	*									if null or less than zero places at end of list
	* @param string			$dependant	'0' / '1' on whether the this asset is dependant on the
	*									asset that will be linked by this new link
	* @param string			$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*									to the asset that will be linked by this new link
	*
	* @return int
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0')
	{
		// if this is a file then we need to make it a dependant link
		if (is_a($minor, 'file')) $dependant = '1';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$linkid = parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// special processing if we are creating a TYPE_2 linked file (a file inherited by customisations)
		if (is_a($minor, 'file')) {

			$am = &$GLOBALS['SQ_SYSTEM']->am;

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
			foreach($customisation_links as $link) {
				$customisation = &$am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_2, 'file', false);
				foreach ($file_links as $link) $file_link_ids[] = $link['minorid'];
				$file_link_info = $am->getAssetInfo(array_unique($file_link_ids));

				// is there already a direct file with the same name
				$insert_link = true;
				foreach ($file_link_info as $info) if ($info['name'] == $minor->name) $insert_link = false;

				if ($insert_link) {
					if (!$customisation->createLink($minor, SQ_LINK_TYPE_3, '', null, 1)) {
						trigger_error('Failed creating file link to customisation', E_USER_WARNING);
						$am->forgetAsset($customisation);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
				}

				$am->forgetAsset($customisation);

			}//end foreach customisation

		}//end if is_a file

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end createLink()


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

		// special processing if we are deleting a TYPE_2 linked file (a file overridding a design file)
		if (is_a($minor, 'file')) {

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
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
		}

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deleteLink()


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
	* Returns a design area for the given id_name
	*
	* @param string	$id_name	the id of the design area to get
	*
	* @return mixed object Design_Area or null
	* @access public
	*/
	function &getDesignArea($id_name)
	{
		if (empty($this->_tmp['design_areas'][$id_name])) {
			if (!isset($this->_tmp['design_areas'])) $this->_tmp['design_areas'] = Array();
			$link = $this->getDesignAreaLink($id_name);
			if (empty($link)) {
				$this->_tmp['design_areas'][$id_name] = null;
			} else {
				$this->_tmp['design_areas'][$id_name] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			}
		}
		return $this->_tmp['design_areas'][$id_name];

	}//end getDesignArea()


	/**
	* Returns the parent design/design_customisation that this customisation is customising
	*
	* @return mixed object Design or null
	* @access public
	*/
	function &getParentDesign()
	{
		// a design is its own customisation
		return $this;

	}//end getParentDesign()


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
		switch($_POST['asset_action']) {
			case 'edit' :
				$this->_tmp['update_customisations'] = false;
				if (parent::processBackend($o, $link)) {
					// regenerate the design, 
					// but don't do the customisations if they have yet to be updated 
					// (they will be regenerated during the update process anyway)
					$this->generateDesignFile(!$this->_tmp['update_customisations']);
					if ($this->_tmp['update_customisations']) $this->_updateCustomisations();

					return true;

				}// end if

				return false;

			default :
				return parent::processBackend($o, $link);

		}// end switch

	}// end processBackend()


	/**
	* Updates each of our customisations from ourselves
	*
	* @return void
	* @access private
	*/
	function _updateCustomisations()
	{
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		foreach($customisation_links as $link) {
			$customisation = &$am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($customisation)) continue;
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($customisation->id)) {
				$customisation->updateFromParent($this);
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($customisation->id);
			}
		}// end foreach

	}// end _updateCustomisations()


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
			case 'html_header' :
				echo '
				<!--   <?php echo SQ_CONF_SYSTEM_NAME; ?>
					  Content © Copyright <?php echo SQ_CONF_SYSTEM_OWNER; ?>
					Running <?php echo SQ_SYSTEM_LONG_NAME; ?> - <?php echo SQ_SYSTEM_URL; ?>
				   Developed by Squiz  - http://www.squiz.net
				  © Copyright <?php echo date(\'Y\') ?>. All rights reserved.
				  Page generated: <?php echo date(\'d F Y\'); ?>
				-->
				<script language="javascript" type="text/javascript"  src="<?php echo sq_web_path(\'lib\'); ?>/js/general.js"></script>
				<link rel="stylesheet" type="text/css"  href="<?php echo sq_web_path(\'lib\'); ?>/web/css/general.css">

				<?php if (SQ_ROLLBACK_VIEW) $GLOBALS[\'SQ_SYSTEM\']->printRollbackWarning(); ?>
				';

				break;

			case 'html_footer' :
				break;

			case 'site_name' :
			case 'asset_title' :
				echo '"', ucwords(str_replace('_', ' ', $var)), '" NOT PROGRAMMED YET';
				break;

			case 'assetid' :
				echo '<', '?php echo ', $this->_escapeVar('$ASSET->id', $args), '; ?', '>';
				break;

			case 'asset_name' :
				echo '<', '?php echo ', $this->_escapeVar('$ASSET->name', $args), '; ?', '>';
				break;

			case 'asset_short_name' :
				echo '<', '?php echo ', $this->_escapeVar('$ASSET->short_name', $args), '; ?', '>';
				break;

			case 'asset_link' :
				echo '<', '?php echo ', $this->_escapeVar('$ASSET->getHref()', $args), '; ?', '>';
				break;

			case 'current_user' :
				
				if (empty($args['attr'])) $args['attr'] = 'name';
				switch($args['attr']) {
					case 'username'   : 
					case 'first_name' : 
					case 'last_name'  : 
						$attr = 'attr(\''.$args['attr'].'\')';
						break;

					case 'name' :
					default :
						$attr = 'name';

				}// end switch

				$default = (empty($args['default'])) ? '' : $args['default'];

				echo '<', '?php echo ', $this->_escapeVar('($GLOBALS[\'SQ_SYSTEM\']->currentUserId()) ? $GLOBALS[\'SQ_SYSTEM\']->user->'.$attr.' : '.var_export($default, true), $args), '; ?', '>';
				break;


			case 'file_path' :
				echo '<','?php echo $FILE_URLS[\''.$args['filename'].'\'];?', '>';
				break;

			default :
				parent::printVar($var, $args);

		}// end switch

	}//end printVar()


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
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'file', false);
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
		speed_check();
		require_once $file_path;
		speed_check();

	}//end paint()


	/**
	* (Re)Generates the design file
	*
	* @param bool	$do_customisations	Generate design files for any customisations
	*
	* @return void
	* @access public
	*/
	function generateDesignFile($do_customisations=true)
	{
		$edit_fns = $this->getEditFns();
		$edit_fns->createDesignFile($this);

		if ($do_customisations) {
			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach($customisation_links as $link) {
				$customisation = &$am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;
				$customisation->generateDesignFile(true);
			}// end foreach
		}// end if

	}//end generateDesignFile()


	/**
	* Returns the file system path to the uploaded file
	*
	* @param string	$filename	the filename to return the path to
	*
	* @return string
	* @access public
	*/
	function filePath($filename)
	{
		return $this->data_path_public.'/files/'.$filename;

	}//end filePath()


	/**
	* Returns the path that has to be appended to the sq_web_path('data') to
	* get access to the file for this design
	*
	* @param string	$filename	the filename to return the path to
	*
	* @return string
	* @access public
	*/
	function webFileSuffix($filename)
	{
		return 'assets/'.$this->type().'/'.$this->id.'/files/'.$filename;

	}//end webFilesSuffix()


}//end class

?>