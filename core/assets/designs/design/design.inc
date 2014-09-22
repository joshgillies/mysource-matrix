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
* $Id: design.inc,v 1.165 2013/08/23 08:05:15 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.165 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Designs set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;


		// create an empty parse file
		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		if (!is_dir($this->data_path)) {
			create_directory($this->data_path);
		}
		$parse_file = $this->data_path.'/parse.txt';
		touch($parse_file);
		if (!$fv->add($this->data_path_suffix, $parse_file, '')) {
			trigger_localised_error('CORE0157', translate('Failed adding parse file version'), E_USER_WARNING)
		}
		if (!$fv->checkOut($this->data_path_suffix.'/parse.txt', $this->data_path)) {
			trigger_localised_error('CORE0158', translate('Failed checking out latest version of parse file'), E_USER_WARNING)
			return FALSE;
		}

		return $this->makeAndSaveInitialWebPath(strtolower($this->_getName()), $link);

	}//end _createAdditional()


	/**
	* Perform any additional processing required during the cloning of this asset
	*
	* This function is being called by asset_manager after the dependants of asset
	* have been cloned.
	*
	* @param Asset	$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_schemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data',
	*							);
	*							or alternately
	*							Array('all');
	*
	* @return boolean
	* @access public
	*/
	public function cloneComponentsAdditional(Asset $clone, Array $components)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// Clone the parse file
		// ONLY do this for the design to design clone and if it will not be morphed to a customisation
		if (($this->type() == 'design' || $this->type() == 'design_css') && !isset($this->_tmp['custom'])) {
			$orig_parse = $this->data_path.'/parse.txt';
			$clone_parse = $clone->data_path.'/parse.txt';

			// Check the path exists and is accessible (and attempt to create if possible)
			if (!is_dir($clone->data_path) || !is_writable($clone->data_path)) {
				if (!create_directory($clone->data_path)) {
					return FALSE;
				}
			}




			// Copy the parse file across first, then process it, to make sure
			if (copy($orig_parse, $clone_parse)) {
				$edit_fns = $clone->getEditFns();

				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

				// tell the asset to update the customisation at the end of the interface processing
				$clone->_tmp['update_customisations'] = TRUE;
				$clone->_tmp['generate_design'] = TRUE;
				$changes = $edit_fns->parseAndProcessFile($clone);

				// attempt to add the parse file to the repository
				if (!$fv->add($clone->data_path_suffix, $clone_parse, '')) {
					trigger_localised_error('CORE0157', translate('Failed adding parse file version'), E_USER_WARNING)
				}

				// make sure we have the latest version of our file
				if (!$fv->checkOut($clone->data_path_suffix.'/parse.txt', $clone->data_path)) {
					trigger_localised_error('CORE0158', translate('Failed checking out latest version of parse file'), E_USER_WARNING)
					return FALSE;
				}

				if (!$clone->generateDesignFile()){
					return FALSE;
				}

			}//end if
		}//end if

		return parent::cloneComponentsAdditional($clone, $components);

	}//end cloneComponentsAdditional()


	/**
	* Perform any additional processing required during the cloning of components of this asset
	*
	* @return boolean
	* @access public
	*/
	public function cloneDependantAdditional(Asset $clone)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$return_value = TRUE;
		$value = TRUE;

		// Load the web paths for the design's components
		$components = $GLOBALS['SQ_SYSTEM']->am->getChildren($clone->id);
		foreach ($components as $componentid => $component) {
			// set web path for components
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($componentid);

			$initial_path = strtolower($asset->name);
			$valid_paths = make_valid_web_paths(Array($initial_path));
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($clone, $valid_paths, $asset->id, TRUE);
			$GLOBALS['SQ_SYSTEM']->acquireLock($asset->id, 'all');
			$return_value = $asset->saveWebPaths($good_paths);
			$value = (!$return_value) ? FALSE : TRUE;

			$return_value = $asset->updateLookups(FALSE);
			$value = (!$return_value) ? FALSE : TRUE;
			$GLOBALS['SQ_SYSTEM']->releaseLock($asset->id, 'all');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		}//end foreach

		return $value;

	}//end cloneDependantAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'design_css'	=> Array('card'	=> 'M', 'exclusive'	=> FALSE),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'design_customisation'	=> Array('card'	=> 'M', 'exclusive'	=> FALSE),
									'design_css'			=> Array('card'	=> 'M', 'exclusive'	=> FALSE),
									'file'					=> Array('card'	=> 'M', 'exclusive'	=> FALSE),
								   ),
				SQ_LINK_TYPE_3	=> Array(
									'design_area'	=> Array('card'	=> 'M', 'exclusive' => FALSE),
								   ),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{

	}//end printFrontend()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	* @see Asset::moveLinkPos()
	*/
	public function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_localised_error('CORE0168', translate('Links under Designs are system controlled and therefore cannot be moved'), E_USER_WARNING)
		return FALSE;

	}//end moveLinkPos()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns TRUE it it's successful
	*
	* @return void
	* @access private
	*/
	public function linksUpdated()
	{
		parent::linksUpdated();
		if (isset($this->_tmp['design_areas'])) {
			foreach (array_keys($this->_tmp['design_areas']) as $id_name) {
				unset($this->_tmp['design_areas'][$id_name]);
			}
		}

	}//end linksUpdated()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param Asset	$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusively
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a file then we need to make it a dependant link
		$changed = FALSE;
		if ($side_of_link == 'major' && ($asset instanceof File)) {
			if ($dependant != '1') {
				$dependant = '1';
				$changed = TRUE;
			}
			// In order to use mysource_files urls, we need to type 2 link files below us
			if ($link_type == SQ_LINK_TYPE_1) {
				$link_type = SQ_LINK_TYPE_2;
				$changed = TRUE;
			}
		}

		return $changed;

	}//end prepareLink()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param Asset	$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param string	$value		the value that is to be associated with this link
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if NULL or less than zero places at end of list
	* @param string	$dependant	'0' / '1' on whether the this asset is dependant on the
	*							asset that will be linked by this new link
	* @param string	$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*							to the asset that will be linked by this new link
	*
	* @return int
	* @access public
	*/
	public function createLink(Asset $minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0')
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$linkid = parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// special processing if we are creating a TYPE_2 linked file (inherited by customisations)
		if (($minor instanceof File)) {

			$am =& $GLOBALS['SQ_SYSTEM']->am; // this need ref!

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
			foreach ($customisation_links as $link) {
				$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_2, 'file', FALSE);
				foreach ($file_links as $link) {
					$file_link_ids[] = $link['minorid'];
				}
				$file_link_info = $am->getAssetInfo(array_unique($file_link_ids));

				// is there already a direct file with the same name
				$insert_link = TRUE;

				// we need to make it a valid web path to compare properly
				$new_file_name = make_valid_web_paths(Array($minor->attr('name')));
				$new_file_name = array_shift($new_file_name);

				foreach ($file_link_info as $info) {
					if ($info['name'] == $new_file_name) {
						$insert_link = FALSE;
					}
				}

				if ($insert_link) {
					if (!$customisation->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
						trigger_localised_error('CORE0162', translate('Failed creating file link to customisation'), E_USER_WARNING)
						$am->forgetAsset($customisation);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
				}

				$am->forgetAsset($customisation);

			}//end foreach customisation

		}//end if is_a file

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
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
	public function deleteLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_localised_error('CORE0155', sprintf(translate('Existing Link #%s not found, unable to delete link'), $linkid), E_USER_WARNING)
			return FALSE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$minor = $am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// special processing if we are deleting a TYPE_2 linked file (a file overridding a design file)
		if ($minor instanceof File) {

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
			foreach ($customisation_links as $link) {
				$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_3, 'file', FALSE);

				foreach ($file_links as $link) {
					if ($link['minorid'] == $delete_link['minorid']) {
						// found the link to delete
						if (!$customisation->deleteLink($link['linkid'])) {
							trigger_localised_error('CORE0165', translate('Failed deleting file link from customisation'), E_USER_WARNING)
							$am->forgetAsset($customisation);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}
				}// end foreach $file_links

				$am->forgetAsset($customisation);

			}//end foreach customisation
		} //end is_a file

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end deleteLink()


	/**
	* Returns the file asset associated with this design that has the passed filename
	*
	* @param string	$filename	the name of the file to look for
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getAssociatedFile($filename)
	{
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$file_link_ids[] = $link['minorid'];
		}
		$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_unique($file_link_ids));

		foreach ($file_link_info as $assetid => $file_info) {
			if ($file_info['name'] == $filename) {
				return $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $file_info['type_code']);
			}
		}

		$null = NULL; // return by ref
		return $null;

	}//end getAssociatedFile()


	/**
	* Get all the stylesheets attached to this design
	*
	* @return array
	* @access public
	*/
	public function getStylesheets()
	{

		// first get all files which end with .css
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$file_link_ids[] = $link['minorid'];
		}
		$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_unique($file_link_ids));

		$return_values = Array();
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		foreach ($file_link_info as $assetid => $file_info) {
			if (get_file_type($file_info['name']) == 'css') {
				$return_values[$assetid] = $file_info['name'];
			}
		}

		return $return_values;

	}//end getStylesheets()


	/**
	* Returns the link information for the design area with the given id_name or all design area links
	*
	* @param string	$id_name	the id of the design area to get
	*							if blank returns links for all design areas
	*
	* @return mixed array|NULL
	* @access public
	*/
	public function getDesignAreaLink($id_name='')
	{
		if ($id_name) {
			return @$GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, 'design_area', FALSE, $id_name);
		} else {
			// we have to remove the real CSS links, as they are not true design areas
			$all_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'design_area', FALSE);
			$return_links = Array();
			foreach ($all_links as $link) {
				if ($link['minor_type_code'] != 'design_css') {
					$return_links[] = $link;
				}
			}
			return $return_links;
		}

	}//end getDesignAreaLink()


	/**
	* Returns a design area for the given id_name
	*
	* @param string	$id_name	the id of the design area to get
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getDesignArea($id_name)
	{
		if (empty($this->_tmp['design_areas'][$id_name])) {
			if (!isset($this->_tmp['design_areas'])) {
				$this->_tmp['design_areas'] = Array();
			}
			$link = $this->getDesignAreaLink($id_name);

			if (empty($link)) {
				$this->_tmp['design_areas'][$id_name] = NULL;
			} else {
				$this->_tmp['design_areas'][$id_name] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			}
		}
		return $this->_tmp['design_areas'][$id_name];

	}//end getDesignArea()


	/**
	* Returns the parent design/design_customisation that this customisation is customising
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getParentDesign()
	{
		return $this;

	}//end getParentDesign()


	/**
	* Processes a backend submission from this asset, returns TRUE if all OK
	*
	* @param Backend_Outputter	$o		Backend Outputter
	* @param array				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	public function processBackend(Backend_Outputter $o, Array &$link)
	{
		switch ($_POST['asset_action']) {
			case 'edit' :
				if (parent::processBackend($o, $link)) {
					// regenerate the design (if necessary) and update customisations in HIPO
					if (isset($this->_tmp['generate_design']) && $this->_tmp['generate_design']) {
						 $hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
							 $vars = Array(
								'assetid'    =>$this->id,
								   );
							$hh->queueHipo('hipo_job_regenerate_design', $vars);

					}

					return TRUE;

				}// end if

				return FALSE;

			default :
				return parent::processBackend($o, $link);

		}//end switch

	}//end processBackend()


	/**
	* Updates each of our customisations from ourselves
	*
	* @return void
	* @access public
	*/
	public function updateCustomisations()
	{
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
		$am = $GLOBALS['SQ_SYSTEM']->am;
		foreach ($customisation_links as $link) {
			$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($customisation)) continue;
			if ($acquired = $GLOBALS['SQ_SYSTEM']->am->acquireLock($customisation->id, 'all')) {
				$customisation->updateFromParent($this);
				if ($acquired != 2) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($customisation->id, 'all');
				}
			}
		}// end foreach

	}//end updateCustomisations()


	/**
	* Outputs the either as straight HTML or PHP code for the var, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param array	$args	array of any extra arguments that existed in the print command
	*
	* @return void
	* @access public
	*/
	public function printVar($var, $args)
	{

		switch ($var) {
			case 'html_header' :
				if (SQ_CONF_DISABLE_ATTRIBUTION === '0') {
echo '<!--
  Running Squiz Matrix
  Developed by Squiz - http://www.squiz.net
  Squiz, Squiz Matrix, MySource, MySource Matrix and Squiz.net are registered Trademarks of Squiz Pty Ltd
  Page generated: <?php echo date(\'d F Y H:i:s\')."\n"; ?>
-->
';
				}
echo '
<?php
if (!empty($SQ_PRIMARY_URL)) {
	?>
	<script type="text/javascript" src="<?php echo sq_web_path(\'lib\'); ?>/js/JsHttpConnector.js"></script>
	<script type="text/javascript" src="<?php echo $SQ_PRIMARY_URL; ?>/__lib/session/session.php?in_primary=<?php echo (sq_web_path(\'root_url\') == $SQ_PRIMARY_URL) ? 1 : 0; ?>&site_network=<?php echo $SQ_SITE_NETWORK->id; ?>"></script>
	<?php
	if (sq_web_path(\'root_url\') != $SQ_PRIMARY_URL) {
		?>
		<script type="text/javascript">
			start_session_handler("<?php echo sq_web_path(\'lib\'); ?>/session/session.php?site_network=<?php echo $SQ_SITE_NETWORK->id; ?>");
		</script>
		<?php
	}
}//end if primary URL available


if (SQ_IN_LIMBO && !is_null($GLOBALS[\'SQ_SYSTEM\']->backend) && !is_null($GLOBALS[\'SQ_SYSTEM\']->backend->out)) $GLOBALS[\'SQ_SYSTEM\']->backend->out->_paintHeader();
if (SQ_ROLLBACK_VIEW) $GLOBALS[\'SQ_SYSTEM\']->printRollbackWarning();
?>

';
			break;

			case 'css_header' :
echo '/*
  <?php echo SQ_CONF_SYSTEM_NAME."\n"; ?>
  Content (c) Copyright <?php echo SQ_CONF_SYSTEM_OWNER."\n"; ?>
  Running <?php echo SQ_SYSTEM_LONG_NAME; ?> - <?php echo SQ_SYSTEM_URL."\n"; ?>
   Developed by Squiz  - http://www.squiz.net
  (c) Copyright <?php echo date(\'Y\') ?>. All rights reserved.
  Page generated: <?php echo date(\'d F Y H:i:s\')."\n"; ?>
  Warranty key: <?php echo (defined("SQ_LICENCE_WARRANTY_KEY") ? SQ_LICENCE_WARRANTY_KEY : "None")."\n"; ?>
*/

';
			break;

			case 'html_footer' :
			break;

			case 'content_type' :
				if (!empty($args['content_type'])) {
					echo '<', '?php header('.$this->_safeStringLiteral('Content-type: '.$args['content_type']).'); ?', '>';
				}
			break;

			case 'pragma' :
				if (!empty($args['pragma'])) {
					echo '<', '?php header('.$this->_safeStringLiteral('Pragma: '.$args['pragma']).'); ?', '>';
				}
			break;

			case 'content_attachment' :
				if (!empty($args['extension'])) {
					echo '<', '?php $asset_name = htmlspecialchars($ASSET->name).\'.'.$args['extension'].'\'; ?'.'>';
					echo '<'.'?php header("Content-Disposition: attachment; filename=\"$asset_name\""); ?'.'>';
				}
			break;

			case 'site_link' :
				if (!empty($args['full_url']) && in_array($args['full_url'], Array('yes', 'true', '1'))) {
					$res_exp = '$ASSET_LINEAGE[0][\'protocol\'].\'://\'.$ASSET_LINEAGE[0][\'url\']';
				} else {
					$res_exp = 'substr($ASSET_LINEAGE[0][\'url\'], strpos($ASSET_LINEAGE[0][\'url\'], \'/\'))';
				}

				$aqs = (!empty($args['append_query_string']) && in_array($args['append_query_string'], Array('yes', 'true', '1')));
				$sqs = (!empty($args['start_query_string']) && in_array($args['start_query_string'], Array('yes', 'true', '1')));

				if ($aqs) {
					$res_exp .= '.((!empty($_SERVER[\'QUERY_STRING\'])) ? \'?\'.$_SERVER[\'QUERY_STRING\'] : \'\')';
				}
				if ($sqs) {
					if (!$aqs) {
						$res_exp .= '.\'?\'';
					} else {
						$res_exp .= '.((!empty($_SERVER[\'QUERY_STRING\'])) ? \'&\' : \'?\');';
					}
				}
				echo '<', '?php echo '.htmlentities($res_exp, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'; ?', '>';
			break;

			case 'site_name' :
				echo '<', '?php echo htmlspecialchars($ASSET_LINEAGE[0][\'short_name\']); ?', '>';
			break;

			case 'assetid' :
				echo '<', '?php echo ', $this->_escapeVar('$ASSET->id', $args), '; ?', '>';
			break;

			case 'asset_name' :
				$asset_name = $this->_escapeVar('htmlspecialchars($ASSET->name)', $args);
				echo '<', '?php echo ', $asset_name, '; ?', '>';
			break;

			case 'asset_short_name' :
				$asset_short_name = $this->_escapeVar('htmlspecialchars($ASSET->short_name)', $args);
				echo '<', '?php echo ', $asset_short_name, '; ?', '>';
			break;

			case 'asset_link' :
				$link_fn = 'getHref';
				if (!empty($args['full_url'])) {
					switch ($args['full_url']) {
						case 'yes'  :
						case 'true' :
						case '1'    :
							$link_fn = 'getUrl';
						break;
					}
				}

				$aqs = FALSE;
				if (!empty($args['append_query_string'])) {
					switch ($args['append_query_string']) {
						case 'yes'  :
						case 'true' :
						case '1'    :
							$aqs = TRUE;
						break;
					}
				}

				$sqs = FALSE;
				if (!empty($args['start_query_string'])) {
					switch ($args['start_query_string']) {
						case 'yes'  :
						case 'true' :
						case '1'    :
							$sqs = TRUE;
						break;
					}
				}

				$cqs = FALSE;
				if (!empty($args['clean_query_string'])) {
					switch ($args['clean_query_string']) {
						case 'yes'  :
						case 'true' :
						case '1'    :
							$cqs = TRUE;
						break;
					}
				}

				$additional = '$query_string = $_SERVER[\'QUERY_STRING\'];';
				if ($cqs) {
					$additional .= 'if (!empty($query_string)) $query_string = preg_replace("/[?&]?SQ_[^&]+/",\'\',$query_string);';
				}


				if ($aqs) {
					$additional .= 'echo htmlentities((!empty($query_string)) ? \'?\'.$query_string : \'\');'."\n";
				}
				if ($sqs) {
					if (!$aqs) {
						$additional .= 'echo \'?\';';
					} else {
						$additional .= 'echo htmlentities((!empty($query_string)) ? \'&\' : \'?\');';
					}
					$additional .= "\n";
				}
				echo '<', '?php echo htmlentities(', $this->_escapeVar('$ASSET->'.$link_fn.'()', $args), ");\n", $additional, ' ?', '>';
			break;

			case 'current_asset' :
			case 'current_user' :
				if ($var == 'current_user') {
					echo '<'.'?php
					if (!$GLOBALS["SQ_SYSTEM"]->userPublic()) {
					?', '>';
					$base = '$GLOBALS["SQ_SYSTEM"]->user';
				} else {
					$base = '$ASSET';
				}

				if (!empty($args['keyword'])) {
					$args['keyword'] = trim($args['keyword'], '% ');
					echo '<'.'?php
					$keyword_val = '.$base.'->getKeywordReplacement('.$this->_safeStringLiteral($args['keyword']).');
					if ($keyword_val != '.$this->_safeStringLiteral('%'.$args['keyword'].'%').') {
						echo $keyword_val;
					}
					?', '>';
				} else {
					if (empty($args['attr'])) $args['attr'] = 'name';
					switch ($args['attr']) {
						case 'name'   :
						case 'id' :
							echo '<'.'?php
							echo '.$base.'->'.$args['attr'].';
							?', '>';
						break;
						case 'password':
							// not allowed
						break;
						default :
							$attr_name = $this->_safeStringLiteral($args['attr']);
							echo '<'.'?php
							echo @'.$base.'->attr('.$attr_name.');
							?', '>';
						break;

					}// end switch
				}

				if ($var == 'current_user') {
					if (!empty($args['default'])) {
						$default = $this->_safeStringLiteral($args['default']);
						echo '<'.'?php
						} else {
							echo '.$default.';
						?', '>';
					}

					echo '<'.'?php
					}//end if not public user
					?', '>';
				}

			break;

			case 'file_path' :
				echo '<','?php echo $FILE_URLS['.$this->_safeStringLiteral($args['filename']).'];?', '>';
			break;

			case 'css_path' :
				echo '<','?php echo $CSS_URLS['.$this->_safeStringLiteral($args['filename']).'];?', '>';
			break;

			case 'asset_thumbnail' :
					echo '<', '?php ';
					echo '$thumbnail = NULL;'."\n";
					echo 'if ($ASSET->useSystemVersion()) {'."\n";
					echo '    $notice_links = unserialize(file_to_string($ASSET->data_path.\'/.sq_notice_links\'));'."\n";
					echo '    foreach ($notice_links as $link) {'."\n";
					echo '		  if ($link[\'value\'] == \'thumbnail\') {'."\n";
					echo '			  $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
					echo '			  break;'."\n";
					echo '		  }'."\n";
					echo '	  }'."\n";
					echo '} else {'."\n";
					echo '    $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink('.$this->_escapeVar('$ASSET->id', $args).', SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
					echo '    if (!empty($link)) {'."\n";
					echo '        $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
					echo '    }'."\n";
					echo '}'."\n";
					echo 'if ($thumbnail) echo $thumbnail->printImageTag(\'\', \'\', TRUE);'."\n";
					echo '?', '>';
			break;

			case 'asset_thumbnail_url' :
					echo '<', '?php ';
					echo '$thumbnail = NULL;'."\n";
					echo 'if ($ASSET->useSystemVersion()) {'."\n";
					echo '    $notice_links = unserialize(file_to_string($ASSET->data_path.\'/.sq_notice_links\'));'."\n";
					echo '    foreach ($notice_links as $link) {'."\n";
					echo '		  if ($link[\'value\'] == \'thumbnail\') {'."\n";
					echo '			  $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
					echo '			  break;'."\n";
					echo '		  }'."\n";
					echo '	  }'."\n";
					echo '} else {'."\n";
					echo '    $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink('.$this->_escapeVar('$ASSET->id', $args).', SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n;";
					echo '    if (!empty($link)) {'."\n";
					echo '        $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
					echo '    }'."\n";
					echo '}'."\n";
					echo 'if ($thumbnail) echo $thumbnail->getURL('.$this->_escapeVar('$ASSET->getUrl()', $args).');'."\n";
					echo '?', '>';
			break;

			case (preg_match('/^asset_/i', $var) ? $var : !$var):
					echo '<', '?php ';
					echo '$keyword = \'',$var, '\';';
					echo '$replacement = '.$this->_escapeVar('$ASSET->getKeywordReplacement($keyword)', $args).';';
					echo 'if ($replacement == $keyword) {';
					echo '	echo \'\';';
					echo '} else {';
					echo '	echo $replacement;';
					echo '}';
					echo '?', '>';
			break;

			default :
				parent::printVar($var, $args);

		}//end switch

	}//end printVar()


	/**
	* Paints out our design
	*
	* @param Asset	$ASSET		the asset that we are going to print the body for
	* @param array	$FILE_URLS	an array of filename => URL to use to print files in the design
	*							doesnt need to be passed in unless some URLs are overridden by
	*							a customisation of the design
	* @param array	$CSS_URLS	an array of filename => URL to use to print css in the design
	*							doesnt need to be passed in unless some URLs are overridden by
	*							a customisation of the design
	*
	* @return void
	* @access public
	*/
	public function paint(Asset $ASSET, Array $FILE_URLS=Array(), Array $CSS_URLS=Array())
	{
		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);

		// generate an array of all the URLs we are going to need for the files in this design
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'file', FALSE);
		foreach ($file_links as $link) {
			$file_link_ids[] = $link['minorid'];
		}

		$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_unique($file_link_ids));
		$file_link_urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($file_link_ids));

		$system_root_url = sq_web_path('root_url');
		foreach ($file_link_info as $minorid => $file_info) {
			$file_url = $file_link_urls[$minorid];
			if ($file_url == '') {
				$file_url = $system_root_url.'/?a='.$minorid;
			} else if ($this->attr('static_url_versioning')){
				$served_by_apache = strpos($file_url, '/__data/') !== FALSE || (SQ_CONF_STATIC_ROOT_URL && strpos($file_url, SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE);
				if ($served_by_apache) $file_url .= '?v='.$file_info['version'];
			}
			$FILE_URLS[$file_info['name']] = $file_url;
		}

		$DATA_PATH = sq_web_path('data'); // this works because we know that all design areas are not going to be restricted
		$LIB_PATH  = sq_web_path('lib');

		// work out where the design file is
		if (SQ_ROLLBACK_VIEW) {
			// get an older version of our design file
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $this->data_path_suffix.'/design_file.php';
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = $fv->_checkOutCheck($rep_file, NULL, $then);
			$file_path = $info['source_file'];
		} else {
			$file_path = $this->data_path.'/design_file.php';
			if (!file_exists($file_path)) {

				// If the design file does not exist, find from the parent design asset
				$arr['majorid'] = $this->id;
				do {
					$arr = $GLOBALS['SQ_SYSTEM']->am->getLink($arr['majorid'], NULL, 'design', FALSE, NULL,'minor', FALSE);
					if (empty($arr)) {
						trigger_localised_error('CORE0259', sprintf(translate('Cannot find the design file to print the asset, "%s"'), $ASSET->id), E_USER_WARNING)
						break;
					}
					$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($arr['majorid']);
					$file_path = dirname(dirname($design->data_path)).'/'.get_asset_hash($design->id).'/'.$arr['majorid'].'/design_file.php';
				} while(!file_exists($file_path));
			}
		}

		// Adjust the file URLs based on selected reference method
		$this->_adjustFileUrls($FILE_URLS);
		$this->_adjustFileUrls($CSS_URLS);

		// Now we just include the design file to get it to print
		// else
		require $file_path;

		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
	}//end paint()


	/**
	* (Re)Generates the design file
	*
	* @param boolean	$do_customisations	Generate design files for any customisations
	*
	* @return boolean
	* @access public
	*/
	public function generateDesignFile($do_customisations=TRUE)
	{
		$this->_setupVarReferences();
		$pre_str = $this->_getDesignFileHeader();

		$str = '';
		$GLOBALS['DESIGN_BEING_GENERATED'] =& $this;
		ob_start();
		foreach ($this->attr('contents') as $item) {
			if ($item['_type'] == 'HTML') {
				// just some HTML content to echo out
				echo $item['contents'];
			} else if ($item['_type'] == 'TAG') {
				if ($item['operation'] == 'area') {
					// defining a design area
					$da = $this->getDesignArea($item['attributes']['id_name']);
					if (is_null($da)) {
						trigger_localised_error('CORE0234', sprintf(translate('No design area asset with ID name %s was found.  There must have been a problem initialising the asset for this design area.  Area tag ignored.'), $item['attributes']['id_name']), E_USER_WARNING)
					} else {
						$da->printAreaCode();
					}
				} else if ($item['operation'] == 'print') {
					if ((!isset($item['attributes']['id_name'])) || ($item['attributes']['id_name'] == '__global__')) {
						// printing a global var
						$args = $item['attributes'];
						unset($args['id_name']);
						unset($args['var']);
						$this->printVar($item['attributes']['var'], $args);
					} else {
						$da = $this->getDesignArea($item['attributes']['id_name']);
						if (is_null($da)) {
							trigger_localised_error('CORE0153', sprintf(translate('Design Area "%s" not found, ignoring print operation'), $item['attributes']['id_name']), E_USER_NOTICE)
						} else {
							if (isset($item['attributes']['var'])) {
								// printing a var from a certain design area
								$args = $item['attributes'];
								unset($args['id_name']);
								unset($args['var']);
								$da->printVar($item['attributes']['var'], $args);
							} else {
								// printing a whole design area (which should already have been defined)
								$da->printPaintingCode();
							}
						}
					}
				}
			}//end if Item = TAG
		}//end foreach contents
		$str = str_replace('?'.'><'.'?php', '', ob_get_contents());
		unset($GLOBALS['DESIGN_BEING_GENERATED']);

		// Get rid of emtpy lines in the beginning
		$str = preg_replace('|^(\r?\n)+|', '', $str);

		ob_end_clean();
		if (empty($str)) return FALSE;

		if (!$this->_writeDesignFile($pre_str.$str)) {
			return FALSE;
		}


		if ($do_customisations) {
			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			foreach ($customisation_links as $link) {
				$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;
				$customisation->generateDesignFile(TRUE);
			}
		}

		return TRUE;

	}//end generateDesignFile()


	/**
	* Return asset id of all the customisations of the design
	*
	* @return array
	* @access public
	*/
	public function getAllCustomisations()
	{
		$customisation_assetids = Array();

		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design', FALSE, 'major', 'customisation');
		foreach ($customisation_links as $link) {
			$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!is_null($customisation)) {
				$customisation_assetids[$link['minorid']] = $link['minor_type_code'];
				// Customisation can have further customisations
				$customisation_assetids += $customisation->getAllCustomisations();
			}//end if
		}//end foreach

		return $customisation_assetids;

	}//end getAllCustomisations()


	/**
	* Get the header for the design file
	*
	* @return string
	* @access private
	*/
	protected function _getDesignFileHeader()
	{
		// we are checking to see if there is a global $ASSET_LINEAGE here
		// because we need other assets to be able to modify the lineage, for
		// instance 404 pages.
		return '<?php
		global $ASSET_LINEAGE;
		$am =& $GLOBALS[\'SQ_SYSTEM\']->am;
		if (!isset($ASSET_LINEAGE) || empty($ASSET_LINEAGE)) $ASSET_LINEAGE = @$am->getLineageFromURL(null, null);
		if (empty($ASSET_LINEAGE)) {
			$asset_url = $ASSET->getURL();
			$protocol_pos = strpos($asset_url, "://");
			if ($protocol_pos !== FALSE) {
				$asset_url = substr($asset_url, $protocol_pos + 3);
			}

			$ASSET_LINEAGE = @$am->getLineageFromURL(null, $asset_url);
		}

		global $SQ_SITE_NETWORK, $SQ_PRIMARY_URL;
		$SQ_PRIMARY_URL = \'\';
		$SQ_SITE_NETWORK = NULL;

		if (isset($ASSET_LINEAGE[0])) {
			$site = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($ASSET_LINEAGE[0][\'assetid\'], $ASSET_LINEAGE[0][\'type_code\']);
			if (!is_null($site)) {
				$SQ_SITE_NETWORK = $site->getSiteNetwork();
				if (!is_null($SQ_SITE_NETWORK)) {
					$SQ_PRIMARY_URL = $SQ_SITE_NETWORK->getPrimaryURL();
					if (!empty($SQ_PRIMARY_URL)) {
						if (sq_web_path(\'root_url\') != $SQ_PRIMARY_URL) {
							require_once SQ_LIB_PATH.\'/session/pre_session.php\';
						}
					}
				}
			}
		}

		if (!defined(\'SQ_LIMBO_EDITING\')) define(\'SQ_LIMBO_EDITING\', (SQ_IN_LIMBO && (!isset($_REQUEST[\'ignore_frames\']) || !$_REQUEST[\'ignore_frames\'])));
		?>';

	}//end _getDesignFileHeader()


	/**
	* Write the design file for the parse file given
	*
	* @param string	$str	The parse file contents
	*
	* @return boolean
	* @access private
	*/
	protected function _writeDesignFile($str)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($this->data_path);
		$file = $this->data_path.'/design_file.php';
		$new_file = !file_exists($file);
		if (!string_to_file($str, $file)) {
			return FALSE;
		}

		//// FILE VERSIONING ////

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		if ($new_file) {
			// attempt to add the file to the repository
			if (!$fv->add($this->data_path_suffix, $file, '')) {
				trigger_localised_error('CORE0156', translate('Failed adding design file version'), E_USER_WARNING)
				return FALSE;
			}
		} else {
			// updating existing file
			$file_status = $fv->upToDate($file);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($file, '')) {
					trigger_localised_error('CORE0159', translate('Failed committing design file version'), E_USER_WARNING)
					return FALSE;
				}
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($this->data_path_suffix.'/design_file.php', $this->data_path)) {
			trigger_localised_error('CORE0031', translate('Failed checking out latest version of design file'), E_USER_WARNING)
			return FALSE;
		}

		return TRUE;

	}//end _writeDesignFile()


	/**
	* Set up the var references between DAs in our design
	*
	* @return void
	* @access private
	*/
	protected function _setupVarReferences()
	{
		$design_area_links = $this->getDesignAreaLink();
		$var_ref_values = Array();
		$da_refs = Array(); // REMOVE THIS SOON
		foreach ($design_area_links as $link) {
			$da = $this->getDesignArea($link['value']);
			if (is_null($da)) continue;

			$da_var_refs = $da->getVarReferences();
			for (reset($da_var_refs); NULL !== ($id_name = key($da_var_refs)); next($da_var_refs)) {
				$var_da = $this->getDesignArea($id_name);
				if (is_null($var_da)) continue;
				if (!isset($var_ref_values[$id_name])) {
					$var_ref_values[$id_name] = Array();
				}
				foreach ($da_var_refs[$id_name] as $var) {
					// does this var want the whole design area ?
					if ($var === '') {
						$da_refs[] = $id_name; // REMOVE THIS SOON
					// else just a normal var (make sure we don't have it already)
					} else if (!isset($var_ref_values[$id_name][$var])) {
						$var_ref_values[$id_name][$var] = $var_da->attr($var);

					}//end if

				}//end foreach

			}//end foreach

		}//end foreach

		// OK, now that we have all the var reference values we can just set them for all the DA's
		foreach ($design_area_links as $link) {
			$da = $this->getDesignArea($link['value']);
			if (is_null($da)) continue;
			$da->setVarReferenceValues($var_ref_values);
		}

	}//end _setupVarReferences()

	/*
	* Adjusted the URLs based on the selected file reference method
	*
	* @param array &$file_urls	List of file URLs
	*
	* @return void
	* @access protected
	*/
	protected function _adjustFileUrls(&$file_urls)
	{
		$reference_type = $this->attr('file_reference_method');

		// Given urls are already absoulte URLs, so do nothing for 'absoulte'
		if ($reference_type != 'absoulte') {

			foreach($file_urls as &$url) {
				$parts = parse_url($url);

				// Seperate URL host and remaining components
				$host = array_get_index($parts, 'host', '');
				$part_url = '';
				if (!empty($parts['path'])) {
					$part_url .= $parts['path'];
				}
				if (!empty($parts['query'])) {
					$part_url .= '?'.$parts['query'];
				}
				if (!empty($parts['fragment'])) {
					$part_url .= '#'.$parts['fragment'];
				}

				if ($reference_type == 'scheme_relative') {
					$url = '//'.$host.$part_url;

				} else if ($reference_type == 'domain_relative') {
					$url = $part_url;
				}

			}//end foreach
		}

	}//end _adjustFileUrls()



	/**
	* Delete this asset from the trash
	*
	* Remove all look up DB entries
	*
	* @param boolean	$release_lock	should we realease the lock after deleting
	*
	* @return boolean
	* @access public
	*/
	public function delete($release_lock=TRUE, $check_locked=TRUE)
	{

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		// Delete the look up values
		$sql = 'DELETE FROM sq_ast_lookup_value WHERE value = :assetid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
		MatrixDAL::execPdoQuery($query);


		if (!parent::delete($release_lock, $check_locked)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


		return TRUE;

	}//end delete()



}//end class
?>
