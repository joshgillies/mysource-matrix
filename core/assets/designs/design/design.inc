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
* $Id: design.inc,v 1.104.2.1 2006/02/27 05:50:14 bcaldwell Exp $
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
* @version $Revision: 1.104.2.1 $
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
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set an initial web path
		$initial_path = strtolower($this->name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
		return $this->saveWebPaths($good_paths);

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
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
	function printFrontend()
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
	function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_localised_error('CORE0168', E_USER_WARNING);
		return FALSE;

	}//end moveLinkPos()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns TRUE it it's successful
	*
	* @return boolean
	* @access private
	*/
	function linksUpdated()
	{
		if (!parent::linksUpdated()) return FALSE;
		if (isset($this->_tmp['design_areas'])) {
			foreach (array_keys($this->_tmp['design_areas']) as $id_name) {
				unset($this->_tmp['design_areas'][$id_name]);
			}
		}
		return TRUE;

	}//end linksUpdated()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
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
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a file then we need to make it a dependant link
		$changed = FALSE;
		if ($side_of_link == 'major' && is_a($asset, 'file')) {
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
	* @param object	&$minor		the minor asset that we are linking to
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
	function createLink(&$minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0')
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
		if (is_a($minor, 'file')) {

			$am =& $GLOBALS['SQ_SYSTEM']->am;

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
			foreach ($customisation_links as $link) {
				$customisation =& $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_2, 'file', FALSE);
				foreach ($file_links as $link) {
					$file_link_ids[] = $link['minorid'];
				}
				$file_link_info = $am->getAssetInfo(array_unique($file_link_ids));

				// is there already a direct file with the same name
				$insert_link = TRUE;
				foreach ($file_link_info as $info) {
					if ($info['name'] == $minor->name) {
						$insert_link = FALSE;
					}
				}

				if ($insert_link) {
					if (!$customisation->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
						trigger_localised_error('CORE0162', E_USER_WARNING);
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
	function deleteLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_localised_error('CORE0155', E_USER_WARNING, $linkid);
			return FALSE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$minor =& $am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// special processing if we are deleting a TYPE_2 linked file (a file overridding a design file)
		if (is_a($minor, 'file')) {

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
			foreach ($customisation_links as $link) {
				$customisation =& $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_3, 'file', FALSE);

				foreach ($file_links as $link) {
					if ($link['minorid'] == $delete_link['minorid']) {
						// found the link to delete
						if (!$customisation->deleteLink($link['linkid'])) {
							trigger_localised_error('CORE1065', E_USER_WARNING);
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
	function &getAssociatedFile($filename)
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
	function getStylesheets()
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
	function getDesignAreaLink($id_name='')
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
	function &getDesignArea($id_name)
	{
		if (empty($this->_tmp['design_areas'][$id_name])) {
			if (!isset($this->_tmp['design_areas'])) {
				$this->_tmp['design_areas'] = Array();
			}
			$link = $this->getDesignAreaLink($id_name);

			if (empty($link)) {
				$this->_tmp['design_areas'][$id_name] = NULL;
			} else {
				$this->_tmp['design_areas'][$id_name] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
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
	function &getParentDesign()
	{
		return $this;

	}//end getParentDesign()


	/**
	* Processes a backend submission from this asset, returns TRUE if all OK
	*
	* @param object	&$o		Backend Outputter
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		switch ($_POST['asset_action']) {
			case 'edit' :
				$this->_tmp['update_customisations'] = FALSE;
				if (parent::processBackend($o, $link)) {
					// regenerate the design (if necessary),
					// but don't do the customisations if they have yet to be updated
					// (they will be regenerated during the update process anyway)
					if (isset($this->_tmp['generate_design']) && $this->_tmp['generate_design']) {
						$this->generateDesignFile(!$this->_tmp['update_customisations']);
						if ($this->_tmp['update_customisations']) {
							$this->_updateCustomisations();
						}
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
	* @access private
	*/
	function _updateCustomisations()
	{
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		foreach ($customisation_links as $link) {
			$customisation =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($customisation)) continue;
			if ($acquired = $GLOBALS['SQ_SYSTEM']->am->acquireLock($customisation->id, 'all')) {
				$customisation->updateFromParent($this);
				if ($acquired != 2) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($customisation->id, 'all');
				}
			}
		}// end foreach

	}//end _updateCustomisations()


	/**
	* Outputs the either as straight HTML or PHP code for the var, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param array	$args	array of any extra arguments that existed in the print command
	*
	* @return void
	* @access public
	*/
	function printVar($var, $args)
	{

		switch ($var) {
			case 'html_header' :
echo '<!--   <?php echo SQ_CONF_SYSTEM_NAME."\n"; ?>
	  Content (c) Copyright <?php echo SQ_CONF_SYSTEM_OWNER."\n"; ?>
	Running <?php echo SQ_SYSTEM_LONG_NAME; ?> - <?php echo SQ_SYSTEM_URL."\n"; ?>
   Developed by Squiz  - http://www.squiz.net
  (c) Copyright <?php echo date(\'Y\') ?>. All rights reserved.
  Page generated: <?php echo date(\'d F Y\')."\n"; ?>
-->

<?php
if (!empty($SQ_PRIMARY_URL)) {
	?>
	<script type="text/javascript" src="<?php echo sq_web_path(\'lib\'); ?>/js/JsHttpConnector.js"></script>
	<script type="text/javascript" src="<?php echo sq_web_path(\'lib\'); ?>/session/session.php?in_primary=<?php echo (sq_web_path(\'root_url\') == $SQ_PRIMARY_URL) ? 1 : 0; ?>&site_network=<?php echo $SQ_SITE_NETWORK->id; ?>"></script>
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
echo '/*   <?php echo SQ_CONF_SYSTEM_NAME."\n"; ?>
	  Content (c) Copyright <?php echo SQ_CONF_SYSTEM_OWNER."\n"; ?>
	Running <?php echo SQ_SYSTEM_LONG_NAME; ?> - <?php echo SQ_SYSTEM_URL."\n"; ?>
   Developed by Squiz  - http://www.squiz.net
  (c) Copyright <?php echo date(\'Y\') ?>. All rights reserved.
  Page generated: <?php echo date(\'d F Y\')."\n"; ?>
*/

';
			break;

			case 'html_footer' :
			break;

			case 'content_type' :
				if (!empty($args['content_type'])) {
					echo '<', '?php header('.var_export('Content-type: '.$args['content_type'], 1).'); ?', '>';
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
				echo '<', '?php echo '.htmlentities($res_exp).'; ?', '>';
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
				$additional = '';
				if ($aqs) {
					$additional .= 'echo htmlentities((!empty($_SERVER[\'QUERY_STRING\'])) ? \'?\'.$_SERVER[\'QUERY_STRING\'] : \'\');'."\n";
				}
				if ($sqs) {
					if (!$aqs) {
						$additional .= 'echo \'?\';';
					} else {
						$additional .= 'echo htmlentities((!empty($_SERVER[\'QUERY_STRING\'])) ? \'&\' : \'?\');';
					}
					$additional .= "\n";
				}
				echo '<', '?php echo htmlentities(', $this->_escapeVar('$ASSET->'.$link_fn.'()', $args), ");\n", $additional, ' ?', '>';
			break;

			case 'current_user' :
				if (empty($args['attr'])) $args['attr'] = 'name';
				switch ($args['attr']) {
					case 'username'   :
					case 'first_name' :
					case 'last_name'  :
						$attr = 'attr('.var_export((string) $args['attr'], TRUE).')';
					break;

					case 'id' :
						$attr = 'id';
					break;

					case 'name' :
						$attr = 'name';
					default :
						// we search the asset for the attributes existance, except for password
						if ($args['attr'] != 'password') {
							$assert_asset_attribute = TRUE;
						}

				}// end switch

				$default = (empty($args['default'])) ? '' : $args['default'];

				// a different string is required if we want an asset attribute
				if (isset($assert_asset_attribute)) {
					echo '<'.'?php
						// check if the attribute we want is an asset specific attribute
						$asset_attributes = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetTypeAttributes(get_class($GLOBALS[\'SQ_SYSTEM\']->user));
						foreach ($asset_attributes as $attribute => $type) {
							if (\''.$args['attr'].'\' == $attribute) {
								echo ', $this->_escapeVar('(!$GLOBALS[\'SQ_SYSTEM\']->userPublic()) ? $GLOBALS[\'SQ_SYSTEM\']->user->attr($attribute) : '.var_export($default, TRUE), $args), ';
								break;
							}
						}
					?'.'>';
				} else {
					echo '<', '?php echo ', $this->_escapeVar('(!$GLOBALS[\'SQ_SYSTEM\']->userPublic()) ? $GLOBALS[\'SQ_SYSTEM\']->user->'.$attr.' : '.var_export($default, TRUE), $args), '; ?', '>';
				}

			break;

			case 'file_path' :
				echo '<','?php echo $FILE_URLS['.var_export((string) $args['filename'], TRUE).'];?', '>';
			break;

			case 'css_path' :
				echo '<','?php echo $CSS_URLS['.var_export((string) $args['filename'], TRUE).'];?', '>';
			break;

			case 'asset_thumbnail' :
					echo '<', '?php ';
					echo '$link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink('.$this->_escapeVar('$ASSET->id', $args).', SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
					echo 'if (!empty($link)) {'."\n";
					echo '$thumbnail =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
					echo 'echo $thumbnail->printImageTag(\'\', \'\', TRUE);'."\n";
					echo '}'."\n";
					echo '?', '>';
			break;

			case 'asset_thumbnail_url' :
					echo '<', '?php ';
					echo '$link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink('.$this->_escapeVar('$ASSET->id', $args).', SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n;";
					echo 'if (!empty($link)) {'."\n";
					echo '$thumbnail =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
					echo 'echo $thumbnail->getURL('.$this->_escapeVar('$ASSET->id', $args).');'."\n";
					echo '}'."\n";
					echo '?', '>';
			break;

			default :
				parent::printVar($var, $args);

		}//end switch

	}//end printVar()


	/**
	* Paints out our design
	*
	* @param object	&$ASSET		the asset that we are going to print the body for
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
	function paint(&$ASSET, $FILE_URLS=Array(), $CSS_URLS=Array())
	{
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
			}
			$FILE_URLS[$file_info['name']] = $file_url;
		}

		$DATA_PATH = sq_web_path('data'); // this works because we know that all design areas are not going to be restricted
		$LIB_PATH  = sq_web_path('lib');

		// work out where the design file is
		if (SQ_ROLLBACK_VIEW) {
			// get an older version of our design file
			$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $this->data_path_suffix.'/design_file.php';
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = $fv->_checkOutCheck($rep_file, NULL, $then);
			$file_path = $info['source_file'];
		} else {
			$file_path = $this->data_path.'/design_file.php';
			if (!file_exists($file_path)) {
				$arr = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, NULL, 'design', TRUE, NULL,'minor', FALSE);
				$file_path = dirname(dirname($this->data_path)).'/design/'.$arr['majorid'].'/design_file.php';
			 }
		}

		// Now we just include the design file to get it to print
		// else
		require_once $file_path;

	}//end paint()


	/**
	* (Re)Generates the design file
	*
	* @param boolean	$do_customisations	Generate design files for any customisations
	*
	* @return void
	* @access public
	*/
	function generateDesignFile($do_customisations=TRUE)
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
					$da =& $this->getDesignArea($item['attributes']['id_name']);
					if (is_null($da)) {
						trigger_localised_error('CORE0234', E_USER_WARNING, $item['attributes']['id_name']);
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
						$da =& $this->getDesignArea($item['attributes']['id_name']);
						if (is_null($da)) {
							trigger_localised_error('CORE0153', E_USER_NOTICE, $item['attributes']['id_name']);
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

		ob_end_clean();
		if (empty($str)) return FALSE;

		if (!$this->_writeDesignFile($pre_str."\n".$str)) {
			return FALSE;
		}


		if ($do_customisations) {
			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			foreach ($customisation_links as $link) {
				$customisation =& $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;
				$customisation->generateDesignFile(TRUE);
			}
		}

	}//end generateDesignFile()


	/**
	* Get the header for the design file
	*
	* @return string
	* @access private
	*/
	function _getDesignFileHeader()
	{
		// we are checking to see if there is a global $ASSET_LINEAGE here
		// because we need other assets to be able to modify the lineage, for
		// instance 404 pages.
		return '<?php
		global $ASSET_LINEAGE;
		$am =& $GLOBALS[\'SQ_SYSTEM\']->am;
		if (!isset($ASSET_LINEAGE) || empty($ASSET_LINEAGE)) $ASSET_LINEAGE = @$am->getLineageFromURL(null, null);

		global $SQ_SITE_NETWORK, $SQ_PRIMARY_URL;
		$SQ_PRIMARY_URL = \'\';
		$SQ_SITE_NETWORK = NULL;

		if (isset($ASSET_LINEAGE[0])) {
			$site =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($ASSET_LINEAGE[0][\'assetid\'], $ASSET_LINEAGE[0][\'type_code\']);
			if (!is_null($site)) {
				$SQ_SITE_NETWORK =& $site->getSiteNetwork();
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

		define(\'SQ_LIMBO_EDITING\', (SQ_IN_LIMBO && (!isset($_REQUEST[\'ignore_frames\']) || !$_REQUEST[\'ignore_frames\'])));
		?>';

	}//end _getDesignFileHeader()


	/**
	* Write the design file for the parse file given
	*
	* @param string	$str	The parse file contents
	*
	* @return void
	* @access private
	*/
	function _writeDesignFile($str)
	{
		$str = preg_replace('|\?>([\s]+)?<\?php|', '', $str);
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($this->data_path);
		$file = $this->data_path.'/design_file.php';
		$new_file = !file_exists($file);
		if (!string_to_file($str, $file)) return FALSE;

		//// FILE VERSIONING ////

		$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		if ($new_file) {
			// attempt to add the file to the repository
			if (!$fv->add($this->data_path_suffix, $file, '')) {
				trigger_localised_error('CORE0156', E_USER_WARNING);
				return FALSE;
			}
		} else {
			// updating existing file
			$file_status = $fv->upToDate($file);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($file, '')) {
					trigger_localised_error('CORE0159', E_USER_WARNING);
					return FALSE;
				}
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($this->data_path_suffix.'/design_file.php', $this->data_path)) {
			trigger_localised_error('CORE0031', E_USER_WARNING);
			return FALSE;
		}

	}//end _writeDesignFile()


	/**
	* Set up the var references between DAs in our design
	*
	* @return void
	* @access private
	*/
	function _setupVarReferences()
	{
		$design_area_links = $this->getDesignAreaLink();
		$var_ref_values = Array();
		$da_refs = Array(); // REMOVE THIS SOON
		foreach ($design_area_links as $link) {
			$da =& $this->getDesignArea($link['value']);
			if (is_null($da)) continue;

			$da_var_refs = $da->getVarReferences();
			for (reset($da_var_refs); NULL !== ($id_name = key($da_var_refs)); next($da_var_refs)) {
				$var_da =& $this->getDesignArea($id_name);
				if (is_null($var_da)) continue;
				if (!isset($var_ref_values[$id_name])) {
					$var_ref_values[$id_name] = Array();
				}
				foreach ($da_var_refs[$id_name] as $var) {
					// does this var want the whole design area ? (It bloody shouldn't)
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
			$da =& $this->getDesignArea($link['value']);
			if (is_null($da)) continue;
			$da->setVarReferenceValues($var_ref_values);
		}

	}//end _setupVarReferences()


}//end class
?>