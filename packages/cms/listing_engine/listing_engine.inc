<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: listing_engine.inc,v 1.360 2013/09/02 04:39:36 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';

/**
* Listing_Engine
*
* Purpose
*	  Provides common listing services to a range of CMS and search
*	  related assets, such as Asset Listing and Search Page. This is
*	  a 'base asset' that should not be created directly - rather you
*	  should create its children
*
*
* @author Luke Wright	<lwright@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @author Marc McIntyre <mmcintyre@squiz.net>
* @author Greg Sherwood <greg@squiz.net>
* @author Andrei Railean <arailean@squiz.net>
* @author Robert Howard <rhoward@squiz.net>
*
* @version $Revision: 1.360 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Listing_Engine extends Page
{


	/**
	* Determines whether the output of this asset should be inside the form
	*
	* @var boolean
	*/
	var $requires_form = FALSE;

	/**
	* @var string suffix to data dir where the performance of listing should be logged
	*/
	var $performance_log_file_name = 'performance';

	/**
	* @var int number of max assets can list without triggering a performance warning
	*/
	var $performance_threshold = 1000;


	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset listings create bopycopy and folder asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
						'type_formats'		=> 'folder',
						'position_formats'	=> 'folder',
						'group_formats'		=> 'folder',
					 );

		$type_formats = NULL;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return FALSE;

			if ($name == 'type_formats') $type_formats = $asset;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$type_formats, 'value' => 'default_format' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', translate('Default Format'));

		$args = Array('content' => $this->_getDefaultBodycopyContent('default_format'));
		if (!$asset->create($copy_link, $args)) return FALSE;

		return $this->_createBodycopies();

	}//end _createAdditional()


	/**
	* Creates bodycopies for this asset
	* Override if different bodycopies are needed (eg. in search page).
	*
	* @return boolean
	* @access protected
	*/
	function _createBodycopies()
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');
		$args = Array('content' => $this->_getDefaultBodycopyContent('page_contents'));
		if (!$asset->create($copy_link, $args)) return FALSE;

		if (!$this->createNoResultsBodycopy(TRUE)) {
			return FALSE;
		}

		return TRUE;

	}//end _createBodycopies()


	/**
	* Get the default content for a bodycopy
	*
	* @param string	$bodycopy_code	The code name for the bodycopy
	*
	* @return string
	* @access private
	*/
	function _getDefaultBodycopyContent($bodycopy_code)
	{
		switch ($bodycopy_code) {
			case 'page_contents':
				return '%asset_listing%';
			case 'no_results':
				return '<p>'.translate('No results were found').'</p>';

			case 'default_format':
				return '%asset_name_linked%';
		}
		return '';

	}//end _getDefaultBodycopyContent()


	/**
	* Returns a list of lock types available for this asset type
	*
	* Only maps Content lock type to the Menu lock type (Attributes|Links) for
	* backward compatibilty purposes with Asset Listing. (@internal If this
	* mapping is not required then this function can be removed)
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = $lock_types['menu'];
		return $lock_types;

	}//end lockTypes()


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
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if a bodycopy is linking to us then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Bodycopy) && $dependant != '1') {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'root' :
				return translate('The root node of the Asset Listing');

			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'type_formats');
		if (!empty($folder_link)) {
			if ($folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
				return sprintf(translate('You cannot delete the link between the type formats folder and the %s page'), str_replace('_',' ', $this->type()));

			}
		}

		$asset_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'position_formats');
		if (!empty($asset_folder_link)) {
			if ($asset_folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
				return sprintf(translate('You cannot delete the link between the position formats folder and the %s page'), str_replace('_',' ', $this->type()));

			}
		}

		$asset_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'group_formats');
		if (!empty($asset_folder_link)) {
			if ($asset_folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
				return sprintf(translate('You cannot delete the link between the group formats folder and the %s page'), str_replace('_',' ', $this->type()));

			}
		}

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		if (!empty($bodycopy_link)) {
			if ($bodycopy_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
				return sprintf(translate('You cannot delete the link between the page contents and the %s page'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));

			}
		}

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'no_results');
		if (empty($bodycopy_link)) {
			// No Results bodycopy is obviously hidden, so we should be getting TYPE_3 link instead
			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, 'bodycopy', TRUE, 'no_results');
		}
		if (!empty($bodycopy_link) && $bodycopy_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
			return sprintf(translate('You cannot delete the link between the page contents and the %s page'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));

		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Perform any additional processing required during the cloning of this asset
	*
	* This function is being called by asset_manager after the dependants of asset
	* have been cloned.
	*
	* @param object	&$clone		the clone asset shell
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
	* @see asset::cloneComponentsAdditional()
	*/
	function cloneComponentsAdditional(&$clone, $components)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (in_array('attributes', $components) || in_array('all', $components)) {

			// assetids of asset grouping bodycopies are sotred an array.
			// They need to be updated the newly cloned bodycopies.
			$asset_grouping = $this->attr('asset_grouping');
			if (!empty($asset_grouping)) {
				$am = $GLOBALS['SQ_SYSTEM']->am;
				$new_asset_grouping = $asset_grouping;
				foreach ($asset_grouping as $key => $group) {
					if (isset($group['format_assetid']) && !empty($group['format_assetid'])) {
						$bodycopy = $am->getAsset($group['format_assetid']);
						$group_format_names[$key] = $bodycopy->name;
					}
				}

				$folder = $clone->getFolder('group_formats');
				if (is_null($folder)) {
					// something went wrong during the dependants cloning
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					trigger_localised_error('CMS0068', E_USER_WARNING);
					return FALSE;
				} else {
					$links_to_bodycopy = $am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
					if (!empty($links_to_bodycopy)) {
						foreach ($links_to_bodycopy as $link) {
							$bodycopy = $am->getAsset($link['minorid']);
							if (($key = array_search($bodycopy->name, $group_format_names)) !== FALSE) {
								$new_asset_grouping[$key]['format_assetid'] = $bodycopy->id;
							}
						}
					}
					$clone->setAttrValue('asset_grouping', $new_asset_grouping);
					$clone->saveAttributes();
				}
			}//end if asset grouping

		}//end if components

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cloneComponentsAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['folder']	= Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['asset']	= Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 2,   'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['bodycopy'] = Array('card' => 1,   'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Return a reference to a folder linked to this asset determined by the passed in value on the link
	*
	* @param string	$type	the value on the link for this folder
	*
	* @return object
	* @access public
	*/
	function &getFolder($type='type_formats')
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $type);
		if (empty($link)) return $null;

		$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return $null;

		return $folder;

	}//end getFolder()


	/**
	* Get a list of all formats of a certain type
	*
	* The results of this function are cached for the length of the script execution
	*
	* @param string	$type	the value on the link for the formats folder
	*
	* @return array
	* @access public
	*/
	function getFormats($type='type_formats')
	{
		if (!isset($this->_tmp['formats'][$type])) {
			$folder = $this->getFolder($type);
			$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE);
			if (empty($format_links)) return Array();

			$formats = Array();
			foreach ($format_links as $link_data) {
				$formats[$link_data['minorid']] = $link_data['value'];
			}
			$this->_tmp['formats'][$type] = $formats;
		}
		return $this->_tmp['formats'][$type];

	}//end getFormats()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// we need a unique cache key to represent the current result page
		$cache_key = $this->_getCacheKey();

		$cached_contents = FALSE;
		if (!empty($cache_key)) {
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$cached_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);
		}

		$group_by = $this->attr('group_by');
		// if the cache is empty, go ahead and regenerate a new version
		if ($cached_contents === FALSE) {

			// get the contents of this page
			// we do this because when contents are prepared the query vars that are used by the form elements
			// need to be unset for us to use them in the form action string
			ob_start();
			$this->printContents();
			$contents = ob_get_clean();

			ob_start();
			$used_fields = $this->getUsedFormFields();

			$selections = $this->attr('asset_selections');

			if (!empty($used_fields) || !empty($selections) || $this->requires_form) {

				// include the 'select all' js if required
				$using_select_all = FALSE;
				foreach ($selections as $selection_name => $selection_settings) {
					if (array_get_index($selection_settings, 'type', NULL) == 'radio' && !empty($selection_settings['options']['allow_multiple'])) {
						?>
						<script type="text/javascript" src="<?php echo sq_web_path('data') ?>/asset_types/listing_engine/js/select_all.js"></script>
						<?php
						$using_select_all = TRUE;
						break;
					}
				}

				$select_all_code = $using_select_all ? $this->getSelectAllJSCodeKeywordReplacement() : '';
				$contents = str_replace('%select_all_js_code%', $select_all_code, $contents);

				if ($this->requires_form || !empty($used_fields)) {
					// Make a clean URL to submit to
					$form_method = strtolower($this->_getFormSubmitMethod());
					$form_url_components = $this->_getFormUrlComponents($used_fields);
					$form_url			 = htmlspecialchars($form_url_components['url']);
					?>

					<form id="<?php echo $this->getPrefix() ?>" method="<?php echo $form_method ?>" action="<?php echo $form_url; ?>">

						<?php
						if ($form_method == 'get') {
							// we couldn't preserve get vars in the form action URL so we add them as hidden fields
							$vars_to_preserve = $_GET;
							foreach ($used_fields as $used_field) {
								if (substr($used_field, -1) == '%') {
									// do a 'like' match
									foreach ($vars_to_preserve as $i => $v) {
										if (0 === strpos($i, substr($used_field, 0, -1))) {
											unset($vars_to_preserve[$i]);
										}
									}
								} else {
									if (isset($vars_to_preserve[$used_field])) {
										unset($vars_to_preserve[$used_field]);
									}
								}
							}
							var_to_hidden_field($vars_to_preserve);
						}
						echo $contents;
						?>

					</form>

					<?php
				} else {

					echo $contents;

				}

			} else {

				// no form at all - remove the JS code relations stuff
				$contents = str_replace('%select_all_js_code%', '', $contents);
				echo $contents;

			}

			if (!empty($cache_key) && (strcmp($group_by, 'random') != 0)) {
				$cm->saveToCache($this->id, $this->type(), $cache_key, ob_get_contents());
			}
			ob_end_flush();

		} else {
			// the contents have already been cached, so just output them
			echo $cached_contents;

		}//end if no cache

	}//end printBody()


	/**
	* Returns the URL that the form will use for its target
	*
	* @return string
	* @access private
	*/
	function _getFormUrlComponents()
	{
		$base_url = ($this->attr('submit_to_page_url') ? $this->getURL() : current_url());
		$form_method = strtolower($this->_getFormSubmitMethod());

		if ($form_method == 'get') {
			// no point retaining query string vars for a GET form action because the browser
			// won't submit them - instead we will preserve them as hidden fields later
			$url = $base_url;
		} else {
			$url = replace_query_string_vars(Array(), $base_url);
			$vars_to_preserve = Array();
		}

		return Array(
				'url'		=> $url,
				'url_base'	=> $base_url,
			   );

	}//end _getFormUrlComponents()


	/**
	* Prints the "No Results found" bodycopy (if available)
	*
	* @param array	&$keywords	a set of keyword replacements to use in the bodycopy
	*
	* @return boolean
	* @access public
	*/
	function printNoResultsBodycopy(&$keywords)
	{
		$no_results_bodycopy_printed = FALSE;
		$no_results_bodycopy = $this->getNoResultsBodycopy();

		// initialise keywords
		$replacements = Array();
		$replacements['asset_count']			= 0;
		$replacements['asset_listing']			= '';
		$replacements['page_list']				= '';
		$replacements['page_number']			= 0;
		$replacements['page_asset_count']		= 0;
		$replacements['total_pages']			= 0;
		$replacements['first_asset_position']	= 0;
		$replacements['last_asset_position']	= 0;
		$replacements['previous_page']			= '';
		$replacements['previous_page_href']		= '';
		$replacements['next_page']				= '';
		$replacements['next_page_href']			= '';

		// print the "No Results" bodycopy if available
		if (!is_null($no_results_bodycopy)) {
			foreach ($keywords as $word) {
				if (isset($replacements[$word])) continue;
				$replacements[$word] = $this->getKeywordReplacement($word);
			}

			// Apply keyword modifiers
			$this->applyKeywordModifier($replacements, $this->id);

			$no_results_bodycopy->setKeywordReplacements($replacements);
			$no_results_bodycopy->printBody();
			$no_results_bodycopy_printed = TRUE;
		}

		return $no_results_bodycopy_printed;

	}//end printNoResultsBodycopy()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		$bodycopy_no_result_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'no_results');
		if (empty($bodycopy_link)) return FALSE;

		$format_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

		if (is_null($format_bodycopy)) return FALSE;

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = $format_bodycopy->getKeywords();
		if ($bodycopy_no_result_link) {
			$format_bodycopy_no_result = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_no_result_link['minorid'], $bodycopy_no_result_link['minor_type_code']);
			$keywords = array_merge($format_bodycopy_no_result->getKeywords(),$keywords);
		}
		$replacements = Array();
		// Timer!
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'getAssetList');
		$children = $this->getAssetList();
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'getAssetList');


		/*
		 * Performance Monitor
		 * Log performance warnings when listing engine tries to list a large number of assets
		 *
		 */
		$total_numbers = isset($children) ? count($children) : 0;
		if ($total_numbers >= $this->performance_threshold) {
			$log_contents = 'Listing page #'.$this->id.' is trying to list '.$total_numbers.' assets, which exceeds the performance monitor threshold of '.$this->performance_threshold;
			log_write($log_contents, $this->performance_log_file_name, E_USER_WARNING);
		}

		$exclude_assets = $this->attr('exclude_assets');

		// get children of exclude root nodes, add them to the exclude list
		$exclude_root_nodes = $this->attr('exclude_root_nodes');
		foreach($exclude_root_nodes as $key =>$value) {
			$exclude_children = $GLOBALS['SQ_SYSTEM']->am->getChildren($value);
			$exclude_assets = array_merge($exclude_assets, array_keys($exclude_children));
		}

		if (!empty($exclude_assets)) {
			foreach ($exclude_assets as $index => $assetid) {
				if (isset($children[$assetid])) {
					unset($children[$assetid]);
				}//end if
			}//end foreach
		}//end if

		if($this->attr('exclude_current_asset')) {
			foreach ($children as $assetid => $data) {
				$url = strip_url($GLOBALS['SQ_SYSTEM']->am->getAssetURL($assetid));
				$requester = strip_url(current_url());
				if ($requester === $url) {
					unset($children[$assetid]);
				}
			}
		}


		$children =& $this->convertProxyAssetTypes($children);
		// Timer!
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'filterAssetList');
		$this->filterAssetList($children);
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'filterAssetList');

		$num_kids = count($children);
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'replaceFormatKeywords');
		foreach ($keywords as $word) {
			$replacements[$word] = $this->getKeywordReplacement($word);
		}

		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'replaceFormatKeywords');


		// if there are no children available, just print the page bodycopy -
		// or the NO RESULTS bodycopy if it exists
		if (empty($children)) {
			$no_results_bodycopy_printed = $this->printNoResultsBodycopy($keywords);

			if (!$no_results_bodycopy_printed) {
				$selections = $this->attr('asset_selections');
				foreach ($selections as $selection_name => $settings) {
					$replacements[$selection_name.'_check_all'] = '';
					$replacements[$selection_name.'_check_all_in_group'] = '';
				}

				$format_bodycopy->setKeywordReplacements($replacements);
				$format_bodycopy->printBody();
			}
			return;
		}


		if ($this->attr('group_by') == 'grouped') {
			// Timer!
			$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'getAssetsInfo');
			// this flag can be set if getAssetList() function returns different format of array.
			// you can find the example in Search List. Also see getAssetList()'s comment
			if (!isset($this->_tmp['no_asset_info_url_cache'])) {
				$this->_tmp['assets_info']	  = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($children));
				$this->_tmp['asset_urls']	  = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_keys($children));
			}
			$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'getAssetsInfo');

			// Timer!
			$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'groupAssets');
			// the list is sorted and positions set, now we can split to groups
				$this->groupAssetsRecursively($this->attr('asset_grouping'), $children, $children);
				$this->sortGroups($this->attr('asset_grouping'), $children, $children);
			$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'groupAssets');
		}

		// Timer!
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'chunkAssets');

		// Which position in the list to start from
		$start_pos = $this->attr('start_position') - 1;
		if ($start_pos > 0) {
			if ($this->attr('group_by') != 'grouped') {
				$children = $start_pos < count($children) ? array_slice($children, $start_pos, count($children) - $start_pos , TRUE) : Array();
			} else {
				// For groups, set the start position for the individual group listing
				foreach($children as $group_id => $child_group) {
					$children[$group_id] = $start_pos < count($child_group) ? array_slice($child_group, $start_pos, count($child_group) - $start_pos, TRUE) : Array();
				}//end foreach
			}//end else
		}//end if start position

		$num_per_page = $this->_getNumPerPage();

		$asset_result_page_var	 = 'result_'.$this->id.'_result_page';
		$generic_result_page_var = 'result_page';

		// Include any GET variables passed (for instance, from the nesting asset)
		// However, make sure that existing values in _REQUEST are not overwritten
		if (!empty($_GET)) {
			$_REQUEST = array_merge($_GET, $_REQUEST);
		}

		// have [assetid]_result_page take precedence over result_page
		$result_page = 1;
		if (isset($_REQUEST[$generic_result_page_var])) {
			$result_page = (int)$_REQUEST[$generic_result_page_var];
		}
		if (isset($_REQUEST[$asset_result_page_var])) {
			$result_page = (int)$_REQUEST[$asset_result_page_var];
		}

		if ($result_page <= 0) $result_page = 1;

		// get our page chunk here
		$todo =& $this->getChunk($children, $replacements, $keywords, $result_page, $num_per_page);

		// If random grouping, we should now shove fixed positions in (it would
		// normally be in filterAssetList). Note that in this case we must still
		// respect the 'per page' settings - but with only one page in random
		// mode that's easy to take care of
		if ($this->attr('group_by') == 'random') {
			$this->adjustAssetPositions($todo);
			while (count($todo) > max($num_per_page, 1)) {
				array_pop($todo);
			}
		}
		// Timer!
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'chunkAssets');

		// For non-grouped listing, get asset info of chucked assets here
		if ($this->attr('group_by') != 'grouped') {
			$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'getAssetsInfo');
			// this flag can be set if getAssetList() function returns different format of array.
			// you can find the example in Search List. Also see getAssetList()'s comment
			if (!isset($this->_tmp['no_asset_info_url_cache'])) {
				$this->_tmp['assets_info']	  = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($todo));
				$this->_tmp['asset_urls']	  = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_keys($todo));
			}
			$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'getAssetsInfo');
		}

		// Timer!
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'printBody');
		// get any extra keyword replacements from this asset or asset_listing
		$replacements = $this->getContentsKeywordReplacements($keywords) + $replacements;
		$num_assets_showing = count($todo);
		if ($num_assets_showing > 0) {
			ob_start();
				$this->printAssetList($todo);
				$global_contents = ob_get_contents();
			ob_end_clean();
		} else {
			$no_results_bodycopy_printed = $this->printNoResultsBodycopy($keywords);

			if (!$no_results_bodycopy_printed) {
				$selections = $this->attr('asset_selections');
				foreach ($selections as $selection_name => $settings) {
					$replacements[$selection_name.'_check_all'] = '';
					$replacements[$selection_name.'_check_all_in_group'] = '';
				}

				$format_bodycopy->setKeywordReplacements($replacements);
				$format_bodycopy->printBody();
			}
			return;
		}

		// the global replacement to print the asset listing
		$replacements['asset_listing'] = $global_contents;
		if ($this->attr('group_by') != 'grouped') {
			$replacements['asset_count'] = $num_kids;
		} else {
			$total_assets = $this->_arrayCountRecursive($children, count($this->attr('asset_grouping')));
			$replacements['asset_count'] = $total_assets;
			$replacements['unique_asset_count'] = $num_kids;
		}

		$result_page = $replacements['page_number'];
		$replacements['page_asset_count'] = $num_assets_showing;
		// compatibility

		if ($replacements['asset_count'] == 0) {
			$replacements['first_asset_position'] = 0;
		} else if ($num_per_page == 0) {
			$replacements['first_asset_position'] = 1;
		} else {
			$replacements['first_asset_position'] = max(1, $num_per_page * ($result_page - 1) + 1);
		}

		if ($num_per_page == 0) {
			$replacements['last_asset_position'] = $replacements['asset_count'];
		} else {
			$replacements['last_asset_position'] = min($replacements['asset_count'], $num_per_page * $result_page);
		}
		$replacements['select_all_js_code'] = '%select_all_js_code%';

		// Apply keyword modifiers
		$this->applyKeywordModifier($replacements, $this->id);

		// print the contents of page - replacing the global keywords
		$format_bodycopy->setKeywordReplacements($replacements);
		if (!in_array('select_all_js_code', $keywords)) {
			// there is no asset listing keyword in the contents, so we dont
			// need to do all the extra processing for the listing
			echo '%select_all_js_code%';
		}
		$format_bodycopy->printBody();

		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($format_bodycopy);

		// Timer!
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printBody');
	}//end printContents()


	/**
	* Work out the unique cache key to represent the current page
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		$group_by = $this->attr('group_by');
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		// we work out what screen we are on here so we can correctly cache the screen with a unique ID
		// note that we never cache the results of a random asset listing for obvious reasons
		$cache_key = '';

		$asset_result_page_var	 = 'result_'.$this->id.'_result_page';
		$generic_result_page_var = 'result_page';

		switch ($group_by) {
			case 'number' :
				// have [assetid]_result_page take precedence over result_page
				$cache_key = 1;
				if (isset($_REQUEST[$generic_result_page_var])) {
					$cache_key = (int)$_REQUEST[$generic_result_page_var];
				}
				if (isset($_REQUEST[$asset_result_page_var])) {
					$cache_key = (int)$_REQUEST[$asset_result_page_var];
				}
				if ($cache_key <= 0) $cache_key = 1;
			break;

			case 'letter' :
				// have [assetid]_result_page take precedence over result_page
				$cache_key = 'A';
				if (isset($_REQUEST[$generic_result_page_var])) {
					$cache_key = $_REQUEST[$generic_result_page_var];
				}
				if (isset($_REQUEST[$asset_result_page_var])) {
					$cache_key = $_REQUEST[$asset_result_page_var];
				}
			break;
		}

		// cache key takes into account different sorting scenarios
		$cache_key .= serialize($this->getSortInfo());
		$cache_key .= (int)$this->isDescending();

		// if we don't have a cache key from the listing it's because we shouldn't be caching
		if ($cache_key) {
			// as per bug #2918, nested asset lists cache too much, so here is the workaround
			$pmap = unserialize($this->attr('parameter_map'));
			if (isset($pmap['root_node'])) {
				$parameter_map = $this->getAttribute('parameter_map');
				$nodes = $parameter_map->getParameterValue('root_node');
				if (is_array($nodes)) {
					$root_nodes = $nodes;
				} else {
					$root_nodes[] = $nodes;
				}
			} else {
				$root_nodes = $this->getRootNodes(FALSE);
			}

			//as per bug #4608, cache key needs to be unique for nested search pages with dynamic stored_query_session
			if ($this->type() == 'search_page') {
				$pmap = unserialize($this->attr('stored_query_session'));
				if (!empty($pmap)) {
					$pmap = $this->getAttribute('stored_query_session');
					$pmap_params = $pmap->getParameters();
					if (is_array($pmap_params) && !empty($pmap_params)) {
						foreach ($pmap_params as $params) {
							$sq_index = $pmap->getParameterValue($params);
							if (isset($sq_index)) $stored_queries[] = $sq_index;
						}
					}
				}

				if (!empty($stored_queries)) {
					$cache_key .= '-'.serialize($stored_queries);
				}
			}

			// funnelback search page dynamic parameters
			if ($this->type() == 'funnelback_search_page') {
				$pmap = $this->getAttribute('dynamic_parameters');
				if(!empty($pmap)) {
					$pmap_params = $pmap->getParameters();
					foreach ($pmap_params as $params) {
						$dynamic_query = $pmap->getParameterValue($params);
						$cache_key .= '-'.$params.':'.$dynamic_query;
					}
				}
			}

			if (!empty($root_nodes)) {
				$cache_key .= '-'.implode('-', $root_nodes);
			}

			$cache_key .= $this->_getNumPerPage();

			// Add context to the cache key, because all our bodycopies could be
			// customised and we are caching the entire contents of the page
			$cache_key .= '-ctx'.$contextid;
		}

		return $cache_key;

	}//end _getCacheKey()


	/**
	* Get the num of items per page
	*
	* @return int
	* @access protected
	*/
	function _getNumPerPage()
	{
		$num_per_page = $this->attr('num_per_page');

		// Check for keywords in the num_per_page attribute
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		replace_global_keywords($num_per_page);
		$num_per_page_keywords = retrieve_keywords_replacements($num_per_page);
		$replacements = Array();
		foreach ($num_per_page_keywords as $keyword) {
			$replacements[$keyword] = $this->getKeywordReplacement($keyword);
		}//end foreach
		replace_keywords($num_per_page, $replacements);
		settype($num_per_page, 'integer');

		return $num_per_page;

	}//end _getNumPerPage()


	/**
	* Get the list of asset that should be printed
	*
	* This is defined as an abstract method of sorts; it's up to the kids to
	* override this
	* The return array is in the form Array(assetid => type_code) unless
	* we are grouping by letter, in which case the return array is
	* <pre>
	* Array(assetid => Array(
	*						'type_code' => type_code,
	*						'first_letter' => first_letter,
	*						);
	*		);
	* </pre>
	*
	* @return array
	* @access protected
	*/
	function getAssetList()
	{
		return Array();

	}//end getAssetList()


	/**
	* Sort the list of assets
	*
	* @param array	$asset_list	an array of assets to be sorted
	* @param array	$sort_info	sorting information as provided by getSortInfo()
	*
	* @return array
	* @access public
	* @see Listing_Engine::getSortInfo()
	*/
	function sortAssetList($asset_list, $sort_info)
	{
		// If the option to sort is No sorting, just return the asset list here and don't do any sorting
		if (isset($sort_info['name']) && $sort_info['name'] == 'No Sorting') {
			return $asset_list;
		}//end if

		if (count($asset_list) == 1 || empty($sort_info)) {
			return $asset_list;
		}
		if ((!is_array($asset_list)) || (empty($asset_list))) {
			return Array();
		}

		// random sort, shuffle the asset list
		if ($sort_info['type'] == '' && $sort_info['name'] == 'Random') {
			$keys = array_keys($asset_list);
			shuffle($keys);
			$rand_asset_list = Array();
			foreach ($keys as $key) {
				$rand_asset_list[$key] = $asset_list[$key];
			}
			return ($rand_asset_list);
		}

		$skipped_assets = Array();
		$sort_assets = Array();
		$reverse_sort = $this->isDescending();

		// default to a numeric sort until we are proven otherwise
		$sort_numeric = TRUE;

		// check and prepare sorting data
		$is_sort_by_attribute = ($sort_info['type'] == 'asset_attrib');
		$is_sort_by_field	  = ($sort_info['type'] == 'field');
		$is_sort_by_metadata  = ($sort_info['type'] == 'metadata');
		$is_sort_by_keyword   = ($sort_info['type'] == 'keyword');

		$sort_params =& $sort_info['params'];
		$assetids = array_keys($asset_list);

		if ($is_sort_by_attribute) {
			$sort_by_attr = $sort_params['attribute'];
			$sort_by_type_code = $sort_params['type_code'];

			foreach ($this->attr('types') as $type_code => $inherit) {
				// initially we plan not to process this type code
				$process = FALSE;
				if ($type_code == $sort_by_type_code) {
					$process = TRUE;
				} else {
					if ($inherit) {
						// we have to find our closest ancestor with a sort field
						$type_anc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
						array_unshift($type_anc, $type_code);

						foreach ($type_anc as $this_type_code) {
							if ($this_type_code == $sort_by_type_code) {
								$process = TRUE;
								break;
							}
						}
					}
				}

				// get the assetids that may be affected
				$rem_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, $type_code, !$inherit));

				// do we have an attribute to sort on for this type code?
				// if we don't, we'll have to come back to it as a skipped asset

				if ($process) {
					if (isset($sort_params['attr_type']) && !in_array($sort_params['attr_type'], Array('int', 'float'))) {
						$sort_numeric = FALSE;
					}

					$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($sort_by_attr, $sort_by_type_code, $rem_assetids);
					foreach ($rem_assetids as $assetid) {
						if (isset($attr_info[$assetid])) {
							$sort_assets[$assetid] = strtolower($attr_info[$assetid]);

							if (($pos = array_search($assetid, $skipped_assets)) !== FALSE) {
								unset($skipped_assets[$pos]);
							}
						}
					}

					$skipped_assets = array_merge($skipped_assets, array_diff($rem_assetids,array_keys($attr_info)));
				} else {
					// add to the skipped assets array if still in the list
					// of used type codes
					$skipped_assets = array_merge($skipped_assets, $rem_assetids);
				}
			}//end foreach
		} else if ($is_sort_by_metadata) {
			// Sort by Metadata field
			$sort_numeric = FALSE;
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$fieldid = $sort_info['field'];

			if (isset($sort_info['field'])) {
				$fieldid = $sort_info['field'];
			}
			if ($fieldid) {
				$field_default_val = $mm->getMetadataFieldDefaultValue($fieldid);
			} else {
				$field_default_val = '';
			}

			$presentation_value = FALSE;
			if ($this->attr('metadata_sort_type') == 'presentation') {
				$presentation_value = TRUE;
			}

			foreach ($asset_list as $assetid => $type) {
				$meta_field_val = $mm->getMetadataValueByAssetid($assetid,$fieldid,TRUE,$presentation_value);
				$meta_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
				$field_type = $meta_field->type();
				// get value of selection if raw display is required
				if (($field_type === 'metadata_field_hierarchy' || $field_type === 'metadata_field_select') && !$presentation_value)  {
				    $meta_field_val= preg_replace('/\\\;/', ';',  $meta_field->getValueFromKey($meta_field_val));
				}
				$field_value = $field_default_val;
				if (isset($meta_field_val)) {
					$field_value = $meta_field_val;

					// get keyword replacements
					if (preg_match('/^%(.*)%$/',$field_value,$matches)) {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if ($asset->id) {
							$field_value = $asset->getKeywordReplacement($matches[1]);
						}
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					}
				}
				$sort_assets[$assetid] = strtolower($field_value);
			}

		} else if ($is_sort_by_keyword) {
			foreach ($asset_list as $assetid => $type) {
				$tmp_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$replacement = $tmp_asset->getKeywordReplacement($sort_info['keyword']);
				if (!empty($replacement)) {
					$sort_assets[$assetid] = $replacement;
				} else {
					// add assets that does not have this keyword to the bottom of the list
					$skipped_assets[] = $assetid;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($tmp_asset);
			}
			$sort_numeric = FALSE;
		} else {
			$sort_field = $sort_params['field'];
			$sortable_info = $this->getSortableAssetInfo();
			if (isset($sortable_info[$sort_field])) {
				$sort_assets = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, NULL, TRUE, $sort_field);
				for (reset($sort_assets); NULL !== ($assetid = key($sort_assets)); next($sort_assets)) {
					$sort_assets[$assetid] = strtolower($sort_assets[$assetid]);
				}

				if ($sort_field != 'assetid') $sort_numeric = FALSE;
			} else {

				if ($reverse_sort) {
					return array_reverse($asset_list, TRUE);
				}

				// sort field is not defined, return the assetlist
				return $asset_list;
			}

		}

		if ($sort_numeric) {
			uasort($sort_assets, create_function('$a,$b','return '.($reverse_sort ? '$b - $a' : '$a - $b').';'));
		} else {
			if ($reverse_sort) {
				arsort($sort_assets);
			} else {
				asort($sort_assets);
			}
		}

		$assets_to_list = Array();
		$sorted_assets = array_keys($sort_assets);
		foreach ($sorted_assets as $assetid) {
			$assets_to_list[$assetid] = $asset_list[$assetid];
		}

		// add skipped assets to the bottom of the list
		foreach ($skipped_assets as $assetid) {
			$assets_to_list[$assetid] = $asset_list[$assetid];
		}
		return $assets_to_list;

	}//end sortAssetList()


	/**
	* Remove unwanted assets from the todo list
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function filterAssetList(&$todo)
	{
		$group_by = $this->attr('group_by');

		// remove non-live assets that the user doesnt have write access to
		if (!empty($todo)) $this->filterAssetStatuses($todo);
		if (empty($todo)) return;

		// remove unwanted asset types
		switch ($group_by) {
			case 'number' :
			case 'grouped' :
				$this->filterAssetTypes($todo);
				$todo = $this->sortAssetList($todo, $this->getSortInfo());
				$this->adjustAssetPositions($todo);
			break;

			case 'random' :
				// adjusting of asset positions happens in printContents(),
				// after the random chunk is gathered
				$this->filterAssetTypes($todo);
			break;

			case 'letter' :
				$filtered_todo = Array();
				for (reset($todo); NULL !== ($todo_id = key($todo)); next($todo)) {
					$filtered_todo[$todo_id] = Array( 0 => Array( 'type_code' => $todo[$todo_id][0]['type_code']));
				}

				$this->filterAssetTypes($filtered_todo);
				foreach (array_keys($todo) as $todo_id) {
					if (!isset($filtered_todo[$todo_id])) {
						unset($todo[$todo_id]);
					}
				}
				unset($filtered_todo);

				$todo = $this->sortAssetList($todo, $this->getSortInfo());

			break;
		}//end switch

	}//end filterAssetList()


	/**
	* Remove assets with a status that we do not want to show
	*
	* For assets with a status less than LIVE, the asset is loaded and read access is checked.
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function filterAssetStatuses(&$todo)
	{
		if (empty($todo)) return;

		// Sort the real assets from the shadows and bridges
		// We'll do status and permission checks on the bridges and apply
		// those results to their shadow assets.
		$real_assetids = Array();
		$bridge_assetids = Array();
		foreach ($todo as $assetid => $details) {
			if (FALSE !== strpos($assetid, ':')) {
				$bridge_assetids[] = strtok($assetid, ':');
			} else {
				$real_assetids[] = $assetid;
			}
		}

		$wanted_statuses = $this->attr('statuses');
		$db = MatrixDAL::getDb();

		if (!empty($wanted_statuses) && (array_sum($wanted_statuses) != SQ_SC_STATUS_ALL)) {
			// find which of the assets have the right statuses
			foreach (array_unique(array_merge($real_assetids, $bridge_assetids)) as $assetid) {
				$query_assetids[] = MatrixDAL::quote((string) $assetid);
			}
			$in_clauses = Array();
			foreach (array_chunk($query_assetids, 999) as $chunk) {
				$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
			}
			foreach ($wanted_statuses as $index => $status) {
			    $wanted_statuses[$index] = MatrixDAL::quote($status);
			}
			$sql = 'SELECT
						assetid
					FROM
						sq_ast
					WHERE
							('.implode(' OR ', $in_clauses).')
						AND status IN ('.implode(', ', $wanted_statuses).')';

			$status_matches = Array();
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				$status_matches = MatrixDAL::executePdoAssoc($query, 0);
			} catch (Execption $e) {
				throw new Exception($e->getMessage());
			}
		} else {
			// We want all statuses
			$status_matches = array_unique(array_merge($real_assetids, $bridge_assetids));
		}

		if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			// Remove any sub-live assets unless we have effective read access to them
			if (!empty($wanted_statuses)) {
				$may_have_sublives = FALSE;
				foreach ($wanted_statuses as $status) {
					if ($status < SQ_STATUS_LIVE) {
						$may_have_sublives = TRUE;
						break;
					}
				}
			} else {
				$may_have_sublives = TRUE;
			}

			if ($may_have_sublives) {
				$sub_live_assets = Array();
				foreach ($status_matches as $assetid) {
					$query_assetids[] = MatrixDAL::quote((string) $assetid);
				}
				$in_clauses = Array();
				foreach (array_chunk($query_assetids, 999) as $chunk) {
					$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
				}
				$sql = 'SELECT
							assetid, type_code
						FROM
							sq_ast
						WHERE
								('.implode(' OR ', $in_clauses).')
							AND status < :status';
				$sub_live_assets = NULL;
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'status', SQ_STATUS_LIVE);
					$sub_live_assets = MatrixDAL::executePdoGroupedAssoc($query);
				} catch (Exception $e) {
					throw new Exception($e->getMessage());
				}//end try catch

				foreach ($status_matches as $i => $v) {
					if (isset($sub_live_assets[$v])) {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($v, $sub_live_assets[$v][0]['type_code']);
						if (is_null($asset) || !$asset->readAccess()) {
							unset($status_matches[$i]);
						}
					}
				}
			}
		}//end if not root or sysadmin

		// Now that we know who the valid candidates are, weed the rest
		// out of the todo list, treating shadows according to their bridge's status
		foreach (array_keys($todo) as $i) {
			if (FALSE !== strpos($i, ':')) {
				$bridgeid = strtok($i, ':');
				if (!in_array($bridgeid, $status_matches)) {
					unset($todo[$i]);
				}
			} else {
				if (!in_array($i, $status_matches)) unset($todo[$i]);
			}
		}

	}//end filterAssetStatuses()


	/**
	* Remove and/or update the todo list based on the types of the assets in it
	*
	* If an entry in the list is a child of a wanted asset type, its type code is changed
	* to the wanted parent type.  If an entry is not a wanted type or a child thereof,
	* it is removed.  Otherwise it's left alone.
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function filterAssetTypes(&$todo)
	{
		$allowed_types = $this->getAllowedAssetTypeMap();
		if (empty($allowed_types)) {
			$todo = Array();
			return;
		}

		foreach ($todo as $assetid => $type_codes) {
			$type_code = $type_codes[0]['type_code'];
			//#4539 if the type_code empty...try to get it
			if (empty ($type_code)){
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$type_code = $asset->type();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			if (!isset($allowed_types[$type_code])) {
				unset($todo[$assetid]);
			} else {
				$todo[$assetid] = Array('0' => Array(
												'type_code'	=> $allowed_types[$type_code],
											   ),
								  );
			}
		}

	}//end filterAssetTypes()


	/**
	* Get the mapping from assets type to the parent type that is handling presentation
	*
	* Returns the map in array format, with allowed asset type as an index
	* with the parent asset being one of the elements of $this->attr('types');
	* Because presentation (type format) is defined only for each of $this->attr('types'),
	* this helps determine the handler of the presentation for a particular type
	* Also, if a particular asset type is defined as the index, that type is allowed to be displayed
	* Handler Asset Type can be the same as Asset Type. Resulting array is in following format:
	* Array['Asset Type'] = 'Handler Asset Type'
	* where Handler Asset Types are unique
	*
	* @return array
	* @access public
	*/
	function getAllowedAssetTypeMap()
	{
		if (isset($this->_tmp['allowed_asset_map'])) {
			return $this->_tmp['allowed_asset_map'];
		}

		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) return Array();

		$allowed_asset_map = Array();
		foreach ($wanted_types as $allowed_parent_type => $inherit) {
			if (trim($allowed_parent_type) == '') {
				continue;
			} else if (!$inherit) {
				$allowed_asset_map[$allowed_parent_type] = $allowed_parent_type;
			} else {
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($allowed_parent_type, TRUE);
				foreach ($descendants as $allowed_type) {
					if (!isset($allowed_asset_map[$allowed_type])) {
						$allowed_asset_map[$allowed_type] = $allowed_parent_type;
					}
				}
			}
		}

		$this->_tmp['allowed_asset_map'] = $allowed_asset_map;
		return $allowed_asset_map;

	}//end getAllowedAssetTypeMap()


	/**
	* Place assets in hard set list positions
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function adjustAssetPositions(&$todo)
	{
		$asset_positions = $this->attr('asset_positions');
		if (empty($asset_positions)) return;

		// replace the original todo list because we are going to build another one
		$all_assets = $todo;
		$adjusted_positions = Array();
		$allowed_asset_map = $this->getAllowedAssetTypeMap();

		// re-adjustment depends on the fact the asset_positions is oredered by position
		// note: list length can only be extended by placing items inside it, not at the end
		// this means that if the list has N elements, you cannot customize position N+1,
		// unless you first insert something into the middle of the list, which will make it N+1 long
		$maximum_possible_position = count($all_assets);
		foreach ($asset_positions as $position => $asset_data) {
			$id = $asset_data['id'];

			// Check to see whether we have read access to the asset - if we
			// don't (or if the asset no longer exists at all...!), don't use it
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
			if (is_null($asset)) {
				$read_access = FALSE;
			} else {
				$read_access = $asset->readAccess();
			}
			unset($asset);

			if (!$read_access) {
				unset($todo[$id]);
				continue;
			}

			if ($position > $maximum_possible_position) {
				// if this position does not fit, neither will anything else
				break;
			}

			if (!isset($all_assets[$id])) {
				// inserting a new asset
				$maximum_possible_position++;
				$type = $asset_data['type'];
				$all_assets[$id] = array_get_index($allowed_asset_map, $type, $type);
			} else {
				// rearranging existing list
				unset($todo[$id]);
			}

			$adjusted_positions[$position] = $id;
		}//end foreach

		// check if adjustment is still required
		if (empty($adjusted_positions)) return;

		$originally_ordered_ids = array_keys($todo);
		$todo = Array();

		$current_position = 1;
		do {
			if (isset($adjusted_positions[$current_position])) {
				$current_id = $adjusted_positions[$current_position];
				unset($adjusted_positions[$current_position]);
			} else {
				$current_id = array_shift($originally_ordered_ids);
			}

			$todo[$current_id] = array_get_index($all_assets, $current_id);

			$current_position++;
		} while (!empty($originally_ordered_ids) || isset($adjusted_positions[$current_position]));

	}//end adjustAssetPositions()


	/**
	* Gets the specified chunk based on group-by and pagination options
	*
	* @param array	&$children		the list of assets that qualify to be
	*								printed
	* @param array	&$replacements	the current replacements array, which will
	*								be added to by this function
	* @param array	$keywords		Keywords that need replacing
	* @param array	$result_page	result page
	* @param array	$num_per_page	number of assets per page
	* @param array	$group_by		what to group by
	*
	* @return array
	* @access public
	*/
	function &getChunk(&$children, &$replacements, $keywords, $result_page, $num_per_page, $group_by=NULL)
	{
		// grouping by number shows a certain number of results per page (eg. 10 assets per page)
		// grouping by letter makes this template an A-Z listing of assets
		// grouping by 'random' means that we grab X random assets
		// grouping by custom groups means we will follow the user defined group structure
		if (is_null($group_by)) {
			$group_by = $this->attr('group_by');
		}

		$page_list_keywords = Array();
		foreach ($keywords as $keyword) {
			if (preg_match('/page_list_([0-9]+)/', $keyword, $matches)) {
				$page_list_keywords[$matches[1]] = $keyword;
			}
		}

		$this->_tmp['result_page'] = 1;

		$asset_result_page_var	 = 'result_'.$this->id.'_result_page';
		$generic_result_page_var = 'result_page';

		switch ($group_by) {
			case 'number' :
				// do it CHUNK Norris style
				if ($num_per_page > 0) {
					$chunks = array_chunk($children, $num_per_page, TRUE);
				} else {
					$chunks = Array('0' => $children);
				}

				if ($result_page > count($chunks)) {
					$result_page = count($chunks);
				}
				$todo = isset($chunks[($result_page-1)]) ? $chunks[($result_page-1)] : Array();

				// previous page link
				if ($result_page <= 1) {
					$replacements['previous_page'] = $this->attr('hide_no_href_links') ? '' : $this->attr('prev_page_text');
					$replacements['previous_page_href'] = '';
				} else {
					// blank out the 'result_page' query - we can understand it if it's given to us, but we don't want to give it to anyone else
					$replacements['previous_page_href'] = htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $result_page-1), NULL, NULL, TRUE));
					$replacements['previous_page'] = '<a href="'.$replacements['previous_page_href'].'">'.$this->attr('prev_page_text').'</a>';
				}


				// next page link
				if ($result_page >= count($chunks)) {
					$replacements['next_page'] = $this->attr('hide_no_href_links') ? '' : $this->attr('next_page_text');
					$replacements['next_page_href'] = '';
				} else {
					$replacements['next_page_href'] = htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $result_page+1), NULL, NULL, TRUE));
					$replacements['next_page'] = '<a href="'.$replacements['next_page_href'].'">'.$this->attr('next_page_text').'</a>';
				}

				$this->_tmp['result_page'] = (int)$result_page;
				$replacements['total_pages'] = count($chunks);

				$page_list =  $this->_getPageList(count($chunks), $result_page);
				$replacements['page_list'] = $page_list;
				$replacements['page_list_without_unused'] = $page_list;

				foreach ($page_list_keywords as $window_size => $keyword) {
					$replacements[$keyword] = $this->_getPageList(count($chunks), $result_page, $window_size);
				}

				unset($chunks);
			break;


			case 'letter' :
				// other letter here refers to the situation where there are assets to list that do not start with A-Z
				$other_letter_value = $this->attr('other_page_text');

				// '@' symbol is chosen here out of the blue, this is used only as a value of the request var
				$other_letter = '@';
				foreach (range('A','Z') as $one_letter) {
					$all_letters[$one_letter] = $one_letter;
				}

				foreach ($children as $childid => $datas) {
					$data = $datas[0];
					$letter = strtoupper($data['first_letter']);
					if (!isset($all_letters[$letter])) {
						$letter = $other_letter;
					}
					$todo[$letter][$childid] = $data['type_code'];
				}

				// 'other' letter is only displayed if there are assets that start with non A-Z chars
				if (isset($todo[$other_letter])) {
					$all_letters[$other_letter] = $other_letter_value;
				}

				if ($this->attr('a_z_style') == 'single') {
					$page_list = '';
					$page_list_without_unused = '';
					foreach ($all_letters as $one_letter => $text_value) {
						if (isset($todo[$one_letter])) {
							$page_link_format = $this->attr('page_link_format');
							$letter_link = str_replace(Array('%page_number%', '%page_link%'), Array($text_value, '#letter_'.$one_letter), $page_link_format);
							$page_list .= $letter_link;
							$page_list_without_unused .= $letter_link;
						} else {
							$current_page_format  = $this->attr('current_page_format');
							$page_list .= str_replace('%page_number%', $text_value, $current_page_format);
						}
					}

					$this->_tmp['result_page'] = (int)$result_page;
					$replacements['page_list'] = $page_list;
					$replacements['page_list_without_unused'] = $page_list_without_unused;

				} else {

					// here we generate a list of only active letters
					$letters = Array();
					foreach ($all_letters as $one_letter => $text) {
						if (isset($todo[$one_letter])) {
							$letters[] = $one_letter;
						}
					}

					$first_available_letter = $letters[0];

					// have [assetid]_result_page take precedence over result_page
					$result_page = $first_available_letter;
					if (isset($_REQUEST[$generic_result_page_var])) {
						$result_page = $_REQUEST['result_page'];
					}
					if (isset($_REQUEST[$asset_result_page_var])) {
						$result_page = $_REQUEST[$asset_result_page_var];
					}

					// by default, result letter is the first letter
					if (!isset($todo[$result_page])) {
						$result_page = $first_available_letter;
					}

					$this_letter_key = array_search($result_page, $letters);

					// previous page link
					$prev_page_text = $this->attr('prev_page_text');
					$prev_page_link = '';
					if ($this_letter_key > 0) {
						$prev_page_link = htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $letters[$this_letter_key - 1]), NULL, NULL, TRUE));
						$prev_page_text = '<a href="'.$prev_page_link.'">'.$prev_page_text.'</a>';
					}
					$replacements['previous_page'] = $prev_page_text;
					$replacements['previous_page_href'] = $prev_page_link;

					// next page link
					$next_page_text = $this->attr('next_page_text');
					$next_page_href = '';
					if ($this_letter_key < (sizeof($letters) - 1)) {
						$next_page_href = htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $letters[$this_letter_key + 1]), NULL, NULL, TRUE));
						$next_page_text = '<a href="'.$next_page_href.'">'.$next_page_text.'</a>';
					}
					$replacements['next_page'] = $next_page_text;
					$replacements['next_page_href'] = $next_page_href;

					$replacements['total_pages'] = count($letters);
					$this->_tmp['result_page'] = $this_letter_key + 1;

					// list of pages and links to them
					$page_list = '';
					$page_list_without_unused = '';
					foreach ($all_letters as $one_letter => $text_value) {
						if (isset($todo[$one_letter])) {
							$page_link_format			 = $this->attr('page_link_format');
							$letter_link				 = str_replace(Array('%page_number%', '%page_link%'), Array($text_value, htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $one_letter)))), $page_link_format);

							$page_list					.= $letter_link;
							$page_list_without_unused	.= $letter_link;
						} else {
							$current_page_format		 = $this->attr('current_page_format');
							$page_list					.= str_replace('%page_number%', $text_value, $current_page_format);

						}
					}

					$todo = $todo[$result_page];
					$replacements['page_list'] = $page_list;
					$replacements['page_list_without_unused'] = $page_list_without_unused;
				}

			break;

			case 'random' :
				// random does things differently, as there is no pagination. But this is a good place
				// to grab the random asset
				$count = count($children);
				$num_per_page = $this->_getNumPerPage();
				$todo = Array();

				if ($count == 1) {
					$todo = $children;
				} else if ($num_per_page <= 1) {
					// grab a single asset to display
					$random_asset = array_rand($children);
					$todo[$random_asset] = $children[$random_asset];
				} else {
					// this will return a subset of the children, as defined by the num_per_page
					// attribute.
					$random_assets = array_rand($children, min($num_per_page, $count));
					foreach ($random_assets as $value) {
						$todo[$value] = $children[$value];
					}
				}

			break;

			case 'grouped' :
				// we need to do an element count so we can work out the page stuff
				$total_assets = $this->_arrayCountRecursive($children, count($this->attr('asset_grouping')));
				$count = $num_per_page * $result_page;

				// if were showing all, make our count equal to all
				if ($count == 0) {
					$num_per_page = $total_assets;
					$count = $total_assets;
				}

				// get a chunk of the size were after, $todo will have the contents
				// after the function call
				$todo = Array();

				// chunk it hard, baby
				$this->sortGroups($this->attr('asset_grouping'), $children, $children);

				$this->_groupChunk($children, $todo, $count, $num_per_page, count($this->attr('asset_grouping')));

				if ($num_per_page == 0) {
					$num_per_page = count($todo);
					$num_pages = 1;
				} else {
					$chunks = array_chunk($todo, $num_per_page, TRUE);
					$num_pages = ceil($total_assets / $num_per_page);
					// we want the last chunk
					$todo = end($chunks);
				}

				// if the result page is too high, set to last page
				if ($num_per_page) {
					$result_page = min($result_page, ceil($total_assets / $num_per_page));
				}

				// previous page link
				if ($result_page <= 1) {
					$replacements['previous_page'] = $this->attr('hide_no_href_links') ? '' : $this->attr('prev_page_text');
					$replacements['previous_page_href'] = '';
				} else {
					$replacements['previous_page_href'] = htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $result_page-1), NULL, NULL, TRUE));
					$replacements['previous_page'] = '<a href="'.$replacements['previous_page_href'].'">'.$this->attr('prev_page_text').'</a>';
				}

				// next page link
				if ($result_page >= $num_pages) {
					$replacements['next_page'] = $this->attr('hide_no_href_links') ? '' : $this->attr('next_page_text');
					$replacements['next_page_href'] = '';
				} else {
					$replacements['next_page_href'] = htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $result_page+1), NULL, NULL, TRUE));
					$replacements['next_page'] = '<a href="'.$replacements['next_page_href'].'">'.$this->attr('next_page_text').'</a>';
				}

				$this->_tmp['result_page'] = (int)$result_page;
				$replacements['total_pages'] = $num_pages;

				// list of pages and links to them
				$page_list = $this->_getPageList($num_pages, $result_page);
				$replacements['page_list'] = $page_list;
				$replacements['page_list_without_unused'] = $page_list;
				foreach ($page_list_keywords as $window_size => $keyword) {
					$replacements[$keyword] = $this->_getPageList($num_pages, $result_page, $window_size);
				}

			break;

		}//end switch

		$replacements['page_number'] = $this->_tmp['result_page'];

		// do this for compatibility - deprecated. not sure if any clients
		// are using this, since 'current_page' is not one of the advertised keywords
		$replacements['current_page'] = $replacements['page_number'];

		return $todo;

	}//end getChunk()


	/**
	* Get a page list with the supplied params
	*
	* @param int	$total_pages	The total number of pages in the listing
	* @param int	$current_page	The current page number
	* @param int	$window_size	The window size for the page list (optional; if absent, all pages shown)
	*
	* @return string
	* @access protected
	*/
	function _getPageList($total_pages, $current_page, $window_size=NULL)
	{
		$current_page_format = $this->attr('current_page_format');
		$page_link_format = $this->attr('page_link_format');

		if (is_null($window_size)) {
			$start_page = 1;
			$end_page = $total_pages;
		} else {
			$start_page = max(1, min(($current_page - floor($window_size / 2)), $total_pages - $window_size + 1));
			$end_page = min($total_pages, $start_page + $window_size - 1);
		}
		$asset_result_page_var = 'result_'.$this->id.'_result_page';
		$page_list = '';
		for ($i = $start_page; $i <= $end_page; $i++) {
			if ($i == $current_page) {
				$page_list .= str_replace('%page_number%', $i, $current_page_format);
			} else {
				$page_list .= str_replace(Array('%page_number%', '%page_link%'), Array($i, htmlspecialchars(replace_query_string_vars(Array($asset_result_page_var => $i)))), $page_link_format);
			}
		}

		return $page_list;

	}//end _getPageList()


	/**
	* Returns a specified chunk of elements
	*
	* will traverse a multidimensional array and return a chunk equal to the
	* size the user specifies, works for single and multi dimensional arrays
	*
	* @param array	&$todo				pointer to the array we want to traverse
	* @param array	&$chunk				pointer to array we want to fill
	* @param int	&$remaining_count	how many assets in we want our chunk including the chunk we want
	* @param mixed	$num_per_page		the number of assets we want returned
	* @param int	$group_limit		group limit
	*
	* @return void
	* @access private
	*/
	function _groupChunk(&$todo, &$chunk, &$remaining_count, $num_per_page, $group_limit=NULL)
	{
		// keep calling this for each group until we get to the actual assets
		if (($group_limit > 0) || is_null($group_limit)) {
			foreach ($todo as $index => $child) {
				if ($remaining_count >= 0) {
					$this->_groupChunk($todo[$index], $chunk[$index], $remaining_count, $num_per_page, ($group_limit == NULL ? NULL : $group_limit - 1));
				}

				if (empty($chunk[$index])) unset($chunk[$index]);
			}
			return;
		}

		$todo = $this->sortAssetList($todo, $this->getSortInfo());
		foreach ($todo as $index => $value) {
			if (($remaining_count > 0) && ($remaining_count <= $num_per_page)) {
				$chunk[$index] = $value;
			}

			$remaining_count--;
		}

	}//end _groupChunk()


	/**
	* Print the list of assets that we are listing
	*
	* @param array	$todo	an array of assets to list in the same format as the return value of getAssetList()
	*
	* @return void
	* @access public
	* @see getAssetList()
	* @see _printAssetList()
	*/
	function printAssetList($todo)
	{
		if (empty($todo)) return;

		$this->_tmp['formats']		  = Array();
		$this->_tmp['default_format'] = '';
		$this->_tmp['is_default']	  = Array();
		if (empty($this->_tmp['assets_info'])) {
			include_once SQ_FUDGE_PATH.'/general/general.inc';
			$flattened = array_keys(array_flatten($todo, FALSE, TRUE));
			$this->_tmp['assets_info'] = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($flattened);
			$this->_tmp['asset_urls'] = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($flattened);
		}

		$type_folder = $this->getFolder('type_formats');

		$this->_tmp['type_folder_id'] = $type_folder->id;

		$this->_tmp['position_formats'] = Array();
		$position_folder = $this->getFolder('position_formats');
		$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($position_folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE);
		foreach ($format_links as $link_data) {
			$position = substr($link_data['value'], 9);
			$this->_tmp['position_formats'][$position] = $link_data['minorid'];
		}

		// in case it wasn't done beforehand in printContents() or something
		$relations = $this->_analyseCheckAllRelations($todo);

		ob_start();
			$selections = $this->attr('asset_selections');
			if (!empty($selections)) {
				?>
				<script type="text/javascript">
				<!--
					<?php echo $this->_buildRelationsJS(); ?>
				//-->
				</script>
				<?php
			}
		$this->_tmp['js_relation_code'] = ob_get_clean();

		if ($this->attr('group_by') == 'grouped') {
			$this->_printGroups($todo, $this->attr('asset_grouping'));
		} else {
			$this->_printAssetList($todo);
		}

	}//end printAssetList()


	/**
	* Prints groups and their asset listings
	*
	* Treats each group as an asset listing to display, if we're not working
	* with groups then it will just skip and call _printAssetList as if
	* nothing happened
	*
	* @param array	&$todo		Our list of groups/assets to print
	* @param array	$group_info	Current group information we are working with
	*
	* @return void
	* @access private
	*/
	function _printGroups(&$todo, $group_info)
	{
		$pointer = reset($todo);
		if (!empty($group_info)) {
			include_once SQ_FUDGE_PATH.'/general/general.inc';
			// get the group bodycopy relative to this level
			$remaining_groups = array_shift($group_info);
			$format_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($remaining_groups['format_assetid']);
			$format_keywords = $format_bodycopy->getKeywords();
			foreach ($todo as $group_level => $value) {
					// lets buffer whatever contents we need
					$replacements = Array();

					switch ($remaining_groups['group_type']) {

						case 'parent_asset':
							// we need to get info from the parent asset
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($group_level);
							if (isset($asset->var['name'])) {
								$group_name = htmlspecialchars($asset->attr('name'));
							} else {
								$group_name = htmlspecialchars($asset->name);
							}
							foreach ($format_keywords as $format_keyword) {
								$pos = strpos($format_keyword, 'parent');
								if ($pos === 0) {
									$keyword = substr_replace($format_keyword, 'asset', 0, 6);
									$replacements[$format_keyword] = $asset->getKeywordReplacement($keyword);
								}
							}
							$replacements['group_name_linked'] = '<a href="'.$asset->getUrl().'">'.htmlspecialchars($asset->name).'</a>';
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						break;

						case 'metadata':
							// we already worked this information out somewhere else
							$group_name = $group_level;
						break;

						case 'standard_asset':
							// format type codes for printing
							if (isset($remaining_groups['field']) && $remaining_groups['field'] == 'type_code') {
								$group_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($group_level, 'name');
							} else {
								// the field value will have been the index
								$group_name = $group_level;
							}
						break;

						case 'keyword':
							$group_name = $group_level;
						break;

						case 'attribute':
							// oh noes, implementation reserved!
						break;

						default:
							// there is no spoon .....
						break;
					}//end switch remaining groups

					if (isset($this->_tmp['group_info'])) {
						array_push($this->_tmp['group_info'], $group_level);
					} else {
						$this->_tmp['group_info'] = Array($group_level);
					}

					// the selection stuff needs the group_info array popped for
					// each group level selection, so we need a copy for _printAsset()
					// too
					if (isset($this->_tmp['group_levels'])) {
						array_push($this->_tmp['group_levels'], $group_level);
					} else {
						$this->_tmp['group_levels'] = Array($group_level);
					}

					ob_start();
						$this->_printGroups($todo[$group_level], $group_info);
						$group_contents = ob_get_contents();
					ob_end_clean();

					// populate replacements
					$replacements['group_listing']				   = $group_contents;
					$replacements['group_name']					   = $group_name;

					// Prints the parent_contents and parent_contents_raw keywords
					if ((preg_match('|%parent_contents%|is', $format_bodycopy->getRawBodycopyContent())) || (preg_match('|%parent_contents_raw%|is', $format_bodycopy->getRawBodycopyContent()))) {
						$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($group_level);
						ob_start();
							$parent_asset->printBodyWithPaintLayout();
							$replacements['parent_contents'] = ob_get_contents();
						ob_end_clean();
						ob_start();
							$parent_asset->printBody();
							$replacements['parent_contents_raw'] = ob_get_contents();
						ob_end_clean();
					}

					$replacements['group_immed_child_group_count'] = count($todo[$group_level]);
					$replacements['group_total_child_asset_count'] = count(array_flatten($todo[$group_level], TRUE, FALSE));

					// only print the 'check all' checkbox if we're only dealing with checkboxes
					$selections = $this->attr('asset_selections');
					$selection_defaults = $this->getAssetSelectionDefaults();

					foreach ($selections as $selection_name => $settings) {
						if (!in_array($selection_name.'_check_all_in_group', $format_keywords)) {
							continue;
						}
						$this->registerFormField($selection_name.'_check_all_in_group');

						// predefine some commonly used settings, for brevity's sake
						$selection_options = array_get_index($settings, 'options', $selection_defaults['options']);
						$selection_allow_multiple = array_get_index($selection_options, 'allow_multiple', $selection_defaults['options']['allow_multiple']);

						if ((array_get_index($settings, 'type', $selection_defaults['type']) == 'radio') && $selection_allow_multiple) {
							ob_start();
							$onclick = Array();
							$group_cb_id  = 'g_'.implode('_', $this->_tmp['group_info']);
							$control_name = make_valid_html_id($this->_getSelectionFieldNamePrefix($selection_name).str_replace(' ', '_', $group_cb_id));
							check_box($control_name, '1', FALSE, 'sq_listing_check_state(this, \''.$this->_getSelectionFieldNamePrefix($selection_name).'\')');
							$replacements[$selection_name.'_check_all_in_group'] = ob_get_clean();
							$replacements[$selection_name.'_selection_id'] = $control_name;
						}
					}

					// Keywords not evaluated above might be the ones with the modifiers
					// see bug #5808 %globals_X% keywords do not work in group format bodycopies
					// and bug #4734 keyword modifiers not working on group_name keyword
					foreach($format_keywords as $format_keyword) {
						// if it is asset based or globals keyword don't empty string the replacement
						// the replacement for these keywords shall be queried later on
						if (!isset($replacements[$format_keyword]) &&
										(strpos($format_keyword, 'globals_') !== FALSE || strpos($format_keyword, 'asset_') !== FALSE)) {
							// if we havn't got a replacement yet
							// dont blank out the keyword already
							$replacements[$format_keyword] = '%'.$format_keyword.'%';
						} else if (!isset($replacements[$format_keyword])) {
							// blank out the replacement if we are sure it is a %group_*%
							// keyword, as we already have replacement of it from up above
							$replacements[$format_keyword] = '';
						}
					}

					// Apply keyword modifiers
					// Use parent asset to replace keywords in modifiers arguments if possible
					$assetid = isset($asset) ? $asset->id : $this->id;
					$this->applyKeywordModifier($replacements, $assetid);

					array_pop($this->_tmp['group_info']);
					$format_bodycopy->setKeywordReplacements($replacements);
					$format_bodycopy->printBody();
			}//end foreach $todo

		} else {
			// Otherwise we're up to the asset list, we can do the normal listing stuff here
			// Check if we need to top up the asset-info array...
			$extras = array_diff(array_keys($todo), array_keys($this->_tmp['assets_info']));
			if (!empty($extras)) {
				$group_assets = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($todo));
				for (reset($group_assets); NULL !== ($index = key($group_assets)); next($group_assets)) {
					$this->_tmp['assets_info'][$index] = $group_assets[$index];
				}
			}
			$this->_printAssetList($todo);

		}

	}//end _printGroups()


	/**
	* Called by printAssetList to take care of the layout of the asset list
	*
	* This method ensures the list is printed in columns if required, and keeps
	* track of what position in the list we are printing
	*
	* @param array	$todo	an array of assets to list in the same format as the return value of getAssetList()
	* unless we have a single paged A-Z asset listing which is an array of letter grouped assets to print.
	*
	* @return void
	* @access private
	* @see printAssetList()
	* @see _printAsset()
	*/
	function _printAssetList($todo)
	{
		$allowed_types = $this->getAllowedAssetTypeMap();
		$column_layout = $this->attr('column_layout');
		$column_layout_type = $this->attr('column_layout_type');
		$using_groups = $this->attr('group_by') == 'grouped';
		$using_columns = (count($column_layout) > 1 || $column_layout['0'] > 1);
		$num_items = count($todo);

		$list_position = 1;
		$a_z_letter_format = $this->attr('a_z_letter_format');
		if ($this->attr('group_by') == 'letter' && $this->attr('a_z_style') == 'single') {
			if ($using_columns && $column_layout_type === 'table') {

				foreach ($todo as $letter => $assets) {
					$letter_text = $letter === '@' ?  $this->attr('other_page_text') : $letter;
					$anchor = '<a name="letter_'.$letter.'">'.$letter_text.'</a>';
					$anchor = str_replace('%letter_group%', $anchor, $a_z_letter_format);
					echo $anchor;
					echo '<table width="100%">';
					$num_rows = max(array_keys($column_layout)) + 1;
					$num_columns = max(array_values($column_layout));
					$current_row = 1;
					for (reset($assets); NULL !== ($assetid = key($assets));) {
						echo '<tr>';
						$cols_in_row = array_get_index($column_layout, $current_row, $column_layout['0']);
						$each_colspan = (int)($num_columns / $cols_in_row);
						$first_colspan = $each_colspan + $num_columns - ($cols_in_row * $each_colspan);

						for ($c = 1; $c <= $cols_in_row; $c++) {
							$colspan = ($c == 1) ? $first_colspan : $each_colspan;
							?>
							<td <?php echo ($colspan > 1) ? ' colspan="'.$colspan.'"' : ''; ?>>
								<?php
								if ($assetid == NULL) {
									echo '&nbsp;';
								} else {
									$this->_printAsset($assetid, $list_position, $num_items);
									next($assets);
									$assetid = key($assets);
									$list_position++;
								}
								?>
							</td>
							<?php
						}
						echo '</tr>';
						$current_row++;
					}//end for

					echo '</table>';
				}
			}
			//if div is used to generate the column
			else if ($using_columns && $column_layout_type === 'div') {
				foreach ($todo as $letter => $assets) {
					$letter_text = $letter === '@' ?  $this->attr('other_page_text') : $letter;
					$anchor = '<a name="letter_'.$letter.'">'.$letter_text.'</a>';
					$anchor = str_replace('%letter_group%', $anchor, $a_z_letter_format);
					echo $anchor;
					$num_rows = max(array_keys($column_layout)) + 1;
					$num_columns = max(array_values($column_layout));
					$row_class = $this->attr('div_row_class');
					$current_row = 1;

					for (reset($assets); NULL !== ($assetid = key($assets));) {
						echo empty($row_class) ? '<div>' : '<div class="'.$row_class.'">';
						$cols_in_row = array_get_index($column_layout, $current_row, $column_layout['0']);

						for ($c = 1; $c <= $cols_in_row; $c++) {
							if ($assetid != NULL) {
								$this->_printAsset($assetid, $list_position, $num_items);
								next($assets);
								$assetid = key($assets);
								$list_position++;
							}
						}
						$current_row++;
						 echo '</div>';
						 echo "\n\n";
					}//end for
				}

			} else {
				foreach ($todo as $letter => $assets) {
					$letter_text = $letter === '@' ?  $this->attr('other_page_text') : $letter;
					$anchor = '<a name="letter_'.$letter.'">'.$letter_text.'</a>';
					$anchor = str_replace('%letter_group%', $anchor, $a_z_letter_format);
					echo $anchor;
					foreach ($assets as $assetid => $type_code) {
						$this->_printAsset($assetid, $list_position, $num_items);
						$list_position++;
					}
				}
			}

		} else {
			//if table style is used to generate the column
			if ($using_columns && $column_layout_type === 'table') {
				$num_rows = max(array_keys($column_layout)) + 1;
				$num_columns = max(array_values($column_layout));

				$current_row = 1;

				echo '<table width="100%">';

				for (reset($todo); NULL !== ($assetid = key($todo));) {

					echo '<tr>';
					$cols_in_row = array_get_index($column_layout, $current_row, $column_layout['0']);
					$each_colspan = (int)($num_columns / $cols_in_row);
					$first_colspan = $each_colspan + $num_columns - ($cols_in_row * $each_colspan);

					for ($c = 1; $c <= $cols_in_row; $c++) {
						$colspan = ($c == 1) ? $first_colspan : $each_colspan;
						?>
						<td<?php echo ($colspan > 1) ? ' colspan="'.$colspan.'"' : ''; ?>>
							<?php
							if ($assetid == NULL) {
								echo '&nbsp;';
							} else {
								$this->_printAsset($assetid, $list_position, $num_items);
								next($todo);
								$assetid = key($todo);
								$list_position++;
							}
							?>
						</td>
						<?php
					}
					echo '</tr>';
					$current_row++;
				}//end for

				echo '</table>';
			}
			//if div is used to generate the column
			else if ($using_columns && $column_layout_type === 'div') {
				$num_rows = max(array_keys($column_layout)) + 1;
				$num_columns = max(array_values($column_layout));
				$row_class = $this->attr('div_row_class');
				$current_row = 1;

				for (reset($todo); NULL !== ($assetid = key($todo));) {
					echo empty($row_class) ? '<div>' : '<div class="'.$row_class.'">';
					$cols_in_row = array_get_index($column_layout, $current_row, $column_layout['0']);

					for ($c = 1; $c <= $cols_in_row; $c++) {
						if ($assetid != NULL) {
							$this->_printAsset($assetid, $list_position, $num_items);
							next($todo);
							$assetid = key($todo);
							$list_position++;
						}
					}
					$current_row++;
					 echo '</div>';
					 echo "\n\n";
				}//end for

			} else {
				foreach ($todo as $assetid => $type_code) {

					$this->_printAsset($assetid, $list_position, $num_items);
					$list_position++;
				}
			}
		}

	}//end _printAssetList()


	/**
	* Print a shadow asset from the list
	*
	* @param string	$assetid		the ID of the asset we are printing
	* @param array	$bc_keywords	the tyope format body copy keywords that are to be replaced
	*
	* @return array
	* @access private
	* @see printAssetList()
	*/
	function _getShadowAssetKeywordReplacements($assetid, $bc_keywords)
	{
		$keywords = Array();
		$shadow_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		foreach ($bc_keywords as $key => $keyword) {
			// if the keyword is globals_* then don't worry about processing
			// it,it will be taken care of at a higher level towards the end
			if (strpos($keyword, 'globals') !== FALSE) {
				$keywords[$keyword] = '%'.$keyword.'%';
				continue;
			}

			$keywords[$keyword] = $shadow_asset->getKeywordReplacement($keyword);
		}

		return $keywords;

	}//end _getShadowAssetKeywordReplacements()


	/**
	* Print a single asset from the list
	*
	* @param string	$assetid		the ID of the asset we are printing
	* @param int	$list_position	the position of this asset in the list
	* @param int	$num_items		the number of items in the current list
	*
	* @return void
	* @access private
	* @see printAssetList()
	*/
	function _printAsset($assetid, $list_position, $num_items=0)
	{
		// allows for asset listings within asset listings, see #2727
		if (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'])) {
			$prev_list_current_asset_id = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'];
		} else {
			$prev_list_current_asset_id = null;
		}
		$_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'] = $assetid;

		$mm			= NULL;
		$contents	= '';
		$keywords	= Array();
		$customised = $this->getFormats('type_formats');

		$asset_info_fields = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
		$bodycopy = NULL;

		// if a ':' exists in the assetid, then it is a shadow asset
		$is_shadow = strpos($assetid, ':');

		// we can get the type code from the asset info array
		// if the asset info array is empty then there is a possibility that we are trying to print a shadow asset
		// so we load the asset and get its typecode
		$type_code = NULL;
		$info = array_get_index($this->_tmp['assets_info'], $assetid, Array('type_code' => ''));
		if (!empty($info['type_code'])) {
			$type_code = $info['type_code'];
		} else {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$type_code = $asset->type();
		}
		// check to see if there is a specific position format for us
		if (isset($this->_tmp['position_formats'][$list_position])) {
			$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['position_formats'][$list_position], 'bodycopy');
		} else if ($num_items > 0) {
			/*
			* Secondly, check if there is a "negative position" that is applicable.
			* Negative position values are used to dynamically control display of list entries by using an offset from the last list element.
			* For example: -1 is a position format for the last entry, -2 is for the second-last entry.
			*
			* Positive position formats take precedence over "negative" ones.
			* Ok, now back to the code...
			*/
			$negative_offset = -1 + ($list_position - $num_items);
			if (isset($this->_tmp['position_formats'][$negative_offset])) {
				$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['position_formats'][$negative_offset], 'bodycopy');
			}
		}//end else

		// if we can't find a position format
		// search for type format, if we don't already know that default format should be used
		if (is_null($bodycopy) && !in_array($type_code, $this->_tmp['is_default'])) {
			// is there previous information about type format?
			if (isset($this->_tmp['type_formats']) && array_key_exists($type_code, $this->_tmp['type_formats'])) {
				$type_key = array_search($this->_tmp['type_formats'][$type_code], $customised);
				$type_code = $this->_tmp['type_formats'][$type_code];
			} else {
				$parent = NULL;
				$parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);

				if (($type_key = array_search($type_code, $customised)) === FALSE) {
					foreach ($parents as $parent) {
						// any of parent in the custom format?
						if (array_search($parent, $customised) !== FALSE) {
							$this->_tmp['type_formats'][$type_code] = $parent;
							$type_key = array_search($this->_tmp['type_formats'][$type_code], $customised);
							$type_code = $parent;
						}
					}
				}
			}
			if (isset($type_key) && $type_key !== FALSE) {
				$bodycopy = $this->_getTypeFormatBodycopy($type_code);
			}

		}

		// if no bodycopy found, try the default format
		if (is_null($bodycopy)) {
			if (!isset($this->_tmp['default_format_bc'])) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'default_format');
				if ($link) {
					$this->_tmp['default_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
				} else {
					// TODO: localise
					trigger_error('Cannot Find the Default Bodycopy');
					return;
				}
			}
			$bodycopy = $this->_tmp['default_format_bc'];
		}


		// by the time we reach this point we already have a bodycopy for this asset
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		if (isset($this->_tmp['bc_keywords'][$bodycopy->id][$contextid])) {
			$bc_keywords = $this->_tmp['bc_keywords'][$bodycopy->id][$contextid];
		} else {
			$bc_keywords = $bodycopy->getKeywords();
			$this->_tmp['bc_keywords'][$bodycopy->id][$contextid] = $bc_keywords;
		}
		$keywords    = Array();

		// Asset contents keywords that we want contexted versions thereof
		// These will be stored in the keywords array as "keyword^context:contextid"
		// and accessed later on when we replace the keyword.
		$context_bc_keywords      = Array();
		$paint_layouts_to_resolve = Array();
		$all_contexts             = $GLOBALS['SQ_SYSTEM']->getAllContexts();

		// For each keyword starting with 'asset_contents', ensure there is an entry
		// in $bc_keywords for both the plain keyword and any equivalents with modifiers.
		//
		// This way plain replacement works as expected, and the modifiers can be run
		// later in this method
		foreach ($bc_keywords as $kw) {
			if (substr($kw, 0, strlen('asset_contents')) == 'asset_contents') {
				$kw = parse_keyword($kw, $modifiers);
				$contextid = extract_context_modifier($modifiers);

				if (!in_array($kw, $bc_keywords)) {
					array_push($bc_keywords, $kw);
				}

				// Store which context we need to get this keyword for
				if (array_key_exists($kw, $context_bc_keywords) === FALSE) {
					$context_bc_keywords[$kw] = Array();
				}

				if ($contextid === NULL) {
					$context_bc_keywords[$kw]['current'] = NULL;
				} else {
					$context_bc_keywords[$kw][$contextid] = $all_contexts[$contextid]['name'];
				}

				// Contents being generated with a paint layout - work out which ones
				// we have to resolve (to get around bug #4374, which previously limited
				// support to one arbitrary paint layout per bodycopy)
				if (substr($kw, 0, strlen('asset_contents_paint_')) == 'asset_contents_paint_') {
					$paint_layouts_to_resolve[] = substr($kw, strlen('asset_contents_paint_'));
				}
			}
		}

		// if the asset is a shadow asset  then get the keywords separately
		if ($is_shadow !== FALSE) {
			$keywords = $this->_getShadowAssetKeywordReplacements($assetid, $bc_keywords);

			// Body with paint layout applied, if any (inherited from the
			// listing engine asset, usually)
			if (in_array('asset_contents', $bc_keywords)) {
				foreach ($context_bc_keywords['asset_contents'] as $contextid => $context_name) {
					$kw = 'asset_contents';
					if ($context_name !== NULL) {
						$kw .= '^context:'.$context_name;
					}
					if (empty($asset)) {
  	                    $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
  	                }
					$keywords[$kw] = $this->_getAssetContentsBCKeywordReplacement($asset, $contextid);
				}

			}

			// %asset_contents_raw% will print the asset's body WITHOUT a
			// paint layout
			if (in_array('asset_contents_raw', $bc_keywords)) {
				foreach ($context_bc_keywords['asset_contents_raw'] as $contextid => $context_name) {
					$kw = 'asset_contents_raw';
					if ($context_name !== NULL) {
						$kw .= '^context:'.$context_name;
					}
					if (empty($asset)) {
  	                    $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
  	                }
					$keywords[$kw] = $this->_getAssetContentsRawBCKeywordReplacement($asset, $contextid);
				}

			}

		} else {
			$this_needs_asset = FALSE;
			foreach ($bc_keywords as $keyword) {
				if (in_array($keyword, Array('asset_url', 'asset_name_linked', 'asset_short_name_linked')) && !SQ_ROLLBACK_VIEW) {
					continue;
				}
				if (!isset($this->_tmp['assets_info'][$assetid][substr($keyword, 6)])) {
					$this_needs_asset = TRUE;
					break;
				}
			}

			if ($this_needs_asset) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) return;
				foreach ($bc_keywords as $keyword) {
					$keywords[$keyword] = $asset->getKeywordReplacement($keyword);
				}

				// Body with paint layout applied, if any (inherited from the
				// listing engine asset, usually)
				if (array_key_exists('asset_contents', $keywords)) {
					foreach ($context_bc_keywords['asset_contents'] as $contextid => $context_name) {
						$kw = 'asset_contents';
						if ($context_name !== NULL) {
							$kw .= '^context:'.$context_name;
						}

						$keywords[$kw] = $this->_getAssetContentsBCKeywordReplacement($asset, $contextid);
					}

				}

				// Body with paint layout(%asset_contents_paint_ASSETID%) or user-defined paint layout(%asset_contents_paint_NAME%) applied
				foreach ($paint_layouts_to_resolve as $paint_layout) {
					$base_kw = 'asset_contents_paint_'.$paint_layout;
					foreach ($context_bc_keywords[$base_kw] as $contextid => $context_name) {
						$kw = $base_kw;
						if ($context_name !== NULL) {
							$kw .= '^context:'.$context_name;
						}

						$keywords[$kw] = $this->_getAssetContentsPaintBCKeywordReplacement($asset, $paint_layout, $contextid);
					}

				}

				// %asset_contents_raw% will print the asset's body WITHOUT a
				// paint layout
				if (array_key_exists('asset_contents_raw', $keywords)) {
					foreach ($context_bc_keywords['asset_contents_raw'] as $contextid => $context_name) {
						$kw = 'asset_contents_raw';
						if ($context_name !== NULL) {
							$kw .= '^context:'.$context_name;
						}

						$keywords[$kw] = $this->_getAssetContentsRawBCKeywordReplacement($asset, $contextid);
					}

				}

				// stuff carried over results body replacements
				$keywords['asset_url'] = array_get_index($this->_tmp['asset_urls'], $assetid, './?a='.$assetid);
				$keywords['asset_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.htmlspecialchars($asset->name).'</a>';
				$keywords['asset_short_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.htmlspecialchars($asset->short_name).'</a>';

				if (in_array('asset_lineage', $bc_keywords) || in_array('asset_lineage_linked', $bc_keywords)) {
					preg_match('/(http[s]?:\/\/)(.*)/', $keywords['asset_url'], $matches);
					$lineage_array = Array();
					if (isset($matches[2])) {
						$lineage_array = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(NULL, $matches[2]);
					}//end if

					$lineage = '';
					$linked_lineage = '';

					if (!empty($lineage_array)) {
						$show_root = $this->attr('root_in_lineage');
						$show_self = $this->attr('self_in_lineage');
						$first = TRUE;

						$served_by_apache = strpos($matches[2], '/__data/') !== FALSE || (SQ_CONF_STATIC_ROOT_URL && strpos($matches[2], SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE);
						if (!$show_self && !$served_by_apache) {
							array_pop($lineage_array);
						}
						foreach ($lineage_array as $lineage_item) {
							if ($first && !$show_root) {
								$first = FALSE;
								continue;
							}//end if

							if (!$first && $lineage != '') {
								$lineage .= $this->attr('lineage_seperator');
								$linked_lineage .= $this->attr('lineage_seperator');
							}//end if
							$lineage .= $lineage_item['short_name'];
							$linked_lineage .= '<a href="'.$lineage_item['protocol'].'://'.$lineage_item['url'].'">'.htmlspecialchars($lineage_item['short_name']).'</a>';
							$first = FALSE;
						}//end if
					}//end if
					$keywords['asset_lineage'] = htmlspecialchars($lineage);
					$keywords['asset_lineage_linked'] = $linked_lineage;
				}//end if

				// end results body stuff

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$keywords = Array();
				foreach ($this->_tmp['assets_info'][$assetid] as $field => $value) {
					$keywords['asset_'.$field] = htmlspecialchars($value);
				}
				$keywords['asset_url'] = array_get_index($this->_tmp['asset_urls'], $assetid, './?a='.$assetid);
				$keywords['asset_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.$keywords['asset_name'].'</a>';
				$keywords['asset_short_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.$keywords['asset_short_name'].'</a>';

			}//end if needs asset
		}//end else

		$group_keywords = preg_grep('/group[0-9]+.*/', $bc_keywords);
		foreach ($group_keywords as $group_keyword) {
			preg_match('/group([0-9]+)_(.*)/', $group_keyword, $matches);
			if (isset($this->_tmp['group_levels'][$matches[1] - 1])) {
				$parent_id = $this->_tmp['group_levels'][$matches[1] - 1];
				if (assert_valid_assetid($parent_id, '', TRUE, FALSE)) {
					// we have an assetid, so do what we want with the parent
					switch ($matches[2]) {
						case 'link_value':
							$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent_id, $assetid);
							$keywords[$group_keyword] = array_get_index($link, 'value', '');
						break;
						case 'link_type':
						case 'dependant':
						case 'exclusive':
						default:
							// other possible uses
						break;
					}
				}
			}//end if
		}

		$keywords['page_href'] = $this->getHref();

		if ($this->attr('group_by') == 'letter') {
			$keywords['asset_position'] = $list_position;
		} else {
			$keywords['asset_position']  = (array_get_index($this->_tmp, 'result_page', 1) - 1) * $this->_getNumPerPage() + $list_position;
		}

		if (($list_position % 2) == 0) {
			$keywords['asset_odd_even'] = translate('even');

		} else {
			$keywords['asset_odd_even'] = translate('odd');

		}

		// asset selections
		$selections			= $this->attr('asset_selections');
		$selection_defaults = $this->getAssetSelectionDefaults();

		if (is_array($selections) && !empty($selections)) {

			foreach ($selections as $selection_name => $settings) {

				$buffering = in_array($selection_name.'_asset_selection', $bc_keywords);
				$selection_prefix = $this->_getSelectionFieldNamePrefix($selection_name);

				if ($buffering) {
					ob_start();
					$this->registerFormField($selection_prefix);
				}

				// predefine some commonly used settings, for brevity's sake
				$selection_options = array_get_index($settings, 'options', $selection_defaults['options']);
				$selection_restrict = array_get_index($selection_options, 'restrict_range', $selection_defaults['options']['restrict_range']);
				$selection_restrict_enable = array_get_index($selection_restrict, 'enable', $selection_defaults['options']['restrict_range']['enable']);
				$selection_restrict_min = array_get_index($selection_restrict, 'min', $selection_defaults['options']['restrict_range']['min']);
				$selection_restrict_max = array_get_index($selection_restrict, 'max', $selection_defaults['options']['restrict_range']['max']);
				$selection_allow_negative = array_get_index($selection_options, 'allow_negative', $selection_defaults['options']['allow_negative']);
				$selection_allow_multiple = array_get_index($selection_options, 'allow_multiple', $selection_defaults['options']['allow_multiple']);

				// grab the values for the field to display
				$selection_value = $this->getAssetSelectionValue($selection_name, $assetid);

				if ($selection_restrict_enable) {
					$selection_value = ($selection_restrict_min > $selection_value ? $selection_restrict_min : $selection_value);
				}

				switch (array_get_index($settings, 'type', $selection_defaults['type'])) {
					case 'textbox':
						$selection_name_field = $selection_prefix.'['.$assetid.']';
						$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

						if ($buffering) {
							if ($selection_restrict_enable) {
								int_text_box($selection_name_field, $selection_value, $selection_allow_negative, 3, $selection_restrict_min, $selection_restrict_max, '', FALSE, FALSE, 'id="'.$selection_id.'"');
							} else {
								int_text_box($selection_name_field, $selection_value, $selection_allow_negative, 3, NULL, NULL, '', FALSE, FALSE, 'id="'.$selection_id.'"');
							}
						}
					break;

					case 'selection':
						$selection_name_field = $selection_prefix.'['.$assetid.']';
						$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

						if ($buffering) {
							// list contents
							$selection_list = Array();
							if (!empty($settings['options']['restrict_range']['enable'])) {
								for ($ii = $selection_restrict_min; $ii <= $selection_restrict_max; $ii++) {
									$selection_list[$ii] = $ii;
								}
							} else {
								$selection_list = Array('0');
							}

							// check that the value is on the list
							if (!isset($selection_list[$selection_value])) {
								$selection_value = reset($selection_list);
							}

							// list display
							combo_box($selection_name_field, $selection_list, $selection_allow_multiple, $selection_value, 0, 'id="'.$selection_id.'"');
						}
					break;

					case 'radio':
					default:
						if ($selection_allow_multiple) {
							$selection_name_field = $selection_prefix.'['.$assetid.']';
							$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

							if ($buffering) {
								check_box($selection_name_field, '1', ($selection_value > 0 ? TRUE : FALSE), "sq_listing_check_state(this, '".$selection_prefix."')", 'id="'.$selection_id.'"');
								$this->registerFormField($selection_name_field);

								// the flag is used to determine whether a checkbox is
								// present and not ticked, or not present at all
								// only print this once, otherwise it ends up bloating the query string
								if (empty($this->_tmp['asset_selection_checkbox_flag'][$selection_name])) {
										hidden_field('selection_flag_'.$selection_prefix, '1');
										$this->_tmp['asset_selection_checkbox_flag'][$selection_name] = TRUE;
								}
							}
						} else {
							$selection_name_field = $selection_prefix;
							$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

							if ($buffering) {
								radio_button($selection_name_field, $assetid, ($selection_value == $assetid ? TRUE : FALSE), '', 'id="'.$selection_id.'"');
							}
						}
					break;
				}//end switch

				// set selection and its ids keywords
				if ($buffering) {
					$keywords[$selection_name.'_asset_selection'] = ob_get_clean();
				}
				$keywords[$selection_name.'_selection_name'] = $selection_name_field;
				$keywords[$selection_name.'_selection_id'] = $selection_id;
			}//end foreach
		}//end if

		// print asset ID always
		$keywords['asset_assetid'] = $assetid;

		// kept for backwards compatibility
		$keywords['result_no']	=& $keywords['asset_position'];
		$keywords['root_nodes'] = $this->getRootNodesKeywordReplacement();

		$keywords += $this->getExtendedAssetKeywordReplacements();

		// Apply keyword modifiers
		$this->applyKeywordModifier($keywords, $assetid);

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

		if ($prev_list_current_asset_id) {
			$_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'] = $prev_list_current_asset_id;
		} else {
			unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id']);
		}

	}//end _printAsset()


	/**
	 * Get the "asset_contents" bodycopy keyword replacement
	 *
	 * This is the asset contents being printed with the "default" inherited
	 * bodycopy. In most cases, this will be inherited from the listing engine.
	 *
	 * @param Asset		$asset			Asset being painted
	 * @param int		$contextid		Return keyword from a different context.
	 *									Leave NULL (or omit) for current context.
	 *
	 * @return string
	 */
	protected function _getAssetContentsBCKeywordReplacement(Asset $asset, $contextid=NULL)
	{
		$replacement = '';

		// Protect against infinite recursion by blocking re-printing of the
		// Listing Engine asset
		if ($asset->id !== $this->id) {
			// Change to alternate context?
			if ($contextid !== NULL) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
			}

			ob_start();
				$asset->printBodyWithPaintLayout();
			$replacement = ob_get_clean();

			// Restore context, and clean the asset from the cache
			if ($contextid !== NULL) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}

		}//end if asset is not Listing Engine itself

		return $replacement;

	}//end _getAssetContentsBCKeywordReplacement()


	/**
	 * Get the "asset_contents_raw" bodycopy keyword replacement
	 *
	 * This is the contents of the asset without any paint layout applied at all.
	 *
	 * @param Asset		$asset			Asset being painted
	 * @param int		$contextid		Return keyword from a different context.
	 *									Leave NULL (or omit) for current context.
	 *
	 * @return string
	 */
	protected function _getAssetContentsRawBCKeywordReplacement(Asset $asset, $contextid=NULL)
	{
		$replacement = '';

		// Protect against infinite recursion by blocking re-printing of the
		// Listing Engine asset
		if ($asset->id !== $this->id) {
			// Change to alternate context?
			if ($contextid !== NULL) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
			}

			ob_start();
				$asset->printBody();
			$replacement = ob_get_clean();

			// Restore context, and clean the asset from the cache
			if ($contextid !== NULL) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}

		}//end if asset is not Listing Engine itself

		return $replacement;

	}//end _getAssetContentsRawBCKeywordReplacement()


	/**
	 * Get the "asset_contents_paint_*" bodycopy keyword replacement
	 *
	 * The paint layout (represented by "*" above) can be an asset ID of a Paint
	 * Layout Page asset if it exists, or the name of a User Defined paint
	 * layout (as defined on the Lookup Settings page of the painted asset).
	 * If it looks like an asset ID, it will always interpret it as that.
	 *
	 * If the $paint_layout does not resolve to a valid paint layout either way,
	 * an empty string will be returned.
	 *
	 * @param Asset		$asset			Asset being painted
	 * @param string	$paint_layout	Asset ID of a paint layout asset, OR
	 *									name of a User Defined paint layout for
	 *									the painted asset
	 * @param int		$contextid		Return keyword from a different context.
	 *									Leave NULL (or omit) for current context.
	 *
	 * @return string
	 */
	protected function _getAssetContentsPaintBCKeywordReplacement(Asset $asset, $paint_layout, $contextid=NULL)
	{
		$replacement = '';

		// Protect against infinite recursion by blocking re-printing of the
		// Listing Engine asset
		if ($asset->id !== $this->id) {
			// Change to alternate context?
			if (($contextid !== NULL) && ($contextid !== 'current')) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
			}

			//layout code name must not be number, otherwise it will be considered as asset id
			if (!assert_valid_assetid($paint_layout, 'Not valid assetid', TRUE, FALSE)) {
				$paint_layout = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL(preg_replace('/^[^:]*:\/\//', '', $asset->getUrl()), 'paint_layout::user::'.$paint_layout);
			}

			if (!empty($paint_layout) && $GLOBALS['SQ_SYSTEM']->am->assetExists($paint_layout)) {
				ob_start();
					$asset->printBodyWithPaintLayout($paint_layout);
				$replacement = ob_get_clean();
			}

			// Restore context, and clean the asset from the cache
			if (($contextid !== NULL) && ($contextid !== 'current')) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}

		}//end if asset is not Listing Engine itself

		return $replacement;

	}//end _getAssetContentsPaintBCKeywordReplacement()


	/**
	* Get the appropriate type format bodycopy for the specified type code
	*
	* @param string	$type_code	The type code for which we want the type format
	*
	* @return object
	* @access private
	*/
	function &_getTypeFormatBodycopy($type_code)
	{
		if (!isset($this->_tmp['type_format_bcs'][$type_code])) {
			$this->_tmp['type_format_bcs'][$type_code] = NULL;
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, $type_code);
			if ($link) {
				$this->_tmp['type_format_bcs'][$type_code] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				$this->_tmp['is_default'][] = $type_code;
			}
		}
		return $this->_tmp['type_format_bcs'][$type_code];

	}//end _getTypeFormatBodycopy()


	/**
	* Get the prefix to use for input elements relevant to the specified selection
	*
	* @param string	$selection_name	The name of the selection
	*
	* @return string
	* @access protected
	*/
	function _getSelectionFieldNamePrefix($selection_name)
	{
		return str_replace(' ', '_', $selection_name);

	}//end _getSelectionFieldNamePrefix()


	/**
	* Gets the list of available asset info fields that can be used
	*
	* @return array
	* @access public
	*/
	function getSortableAssetInfo()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();

	}//end getSortableAssetInfo()


	/**
	* Gets the list of current root nodes
	*
	* This function has optional caching.  The assumption is that it will
	* be called with $reload=TRUE (the default) any time the root nodes change
	*
	* @param string	$reload	whether to refresh the root node cache
	*
	* @return array
	* @access public
	*/
	function getRootNodes($reload=TRUE)
	{
		$frontend_asset =& $GLOBALS['SQ_SYSTEM']->frontend_asset;
		$frontend_asset_id = (!is_null($frontend_asset)) ? $frontend_asset->id : NULL;
		if ($reload || (!empty($GLOBALS['SQ_SYSTEM']->frontend_asset) && !isset($this->_tmp['root_nodes_'.$frontend_asset_id]))) {
			$root_asset_ids = Array();
			$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');

			foreach ($root_links as $root_link) {
				if (strstr($root_link['minorid'], ':') !== FALSE) {
					$root_asset_ids[] = $root_link['minorid'];
				} else {
					$root_asset_ids[] = (int)$root_link['minorid'];
				}
			}

			$parameter_map = $this->getAttribute('parameter_map');
			$raw_dynamic_root_nodes = $parameter_map->getParameterValue('root_node');
			$empty_dynamic_roots = TRUE;

			// check if we have a dynamic parameter -- if so, let's do some security checks
			if (!empty($raw_dynamic_root_nodes)) {

				$dynamic_root_nodes = Array();
				// note that dynamic roots can be supplied as an array of asset ids or
				// as a comma delimited string of asset ids
				if (!is_array($raw_dynamic_root_nodes)) {
					$raw_dynamic_root_nodes = explode(',', $raw_dynamic_root_nodes);
				}

				foreach ($raw_dynamic_root_nodes as $dynamic_root) {
					if (!strlen($dynamic_root)) continue;

					$dynamic_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($dynamic_root);

					// first, check to see if the dynamic parameter specified is already one of the root nodes
					// if so, just use that single root node
					if (in_array($dynamic_root, $root_asset_ids)) {
						$dynamic_root_nodes[] = $dynamic_root;
					} else {
						// otherwise, check to see if the dynamic parameter specified is a child of one of the root
						// nodes. If so, use the dynamic parameter as the root node; otherwise return an error.
						$matching_root_ids = array_intersect(array_keys($dynamic_parents), $root_asset_ids);
						if (empty($matching_root_ids)) {
							trigger_localised_error('CMS0020', E_USER_WARNING, $dynamic_root, $this->id);
						} else {
							$dynamic_root_nodes[] = $dynamic_root;
						}
					}
				}

				if (!empty($dynamic_root_nodes)) {
					$root_asset_ids = $dynamic_root_nodes;
					$empty_dynamic_roots = FALSE;
				}

			}//end if
			$dynamic_parameters = $parameter_map->getParameters();
			if (in_array('root_node', $dynamic_parameters) && $empty_dynamic_roots && $this->attr('dynamic_root_option') == 'empty_result') {
				$root_asset_ids = Array();
			} else if (empty($root_asset_ids)) {
				$root_asset_ids = Array($this->id);
			}
			$this->_tmp['root_nodes_'.$frontend_asset_id] = $root_asset_ids;
		}//end if


		return isset($this->_tmp['root_nodes_'.$frontend_asset_id]) ? $this->_tmp['root_nodes_'.$frontend_asset_id] : NULL;

	}//end getRootNodes()


	/**
	* Returns a list of extra keywords that should be replaced for a particular
	* listing asset
	*
	* This allows search page, for instance, to define its result number and
	* score fields so they don't blank out when keywords are replaced in listing
	* engine. Return data format: Array(string => string)	keyword name => replacement
	*
	* @return array
	* @access public
	*/
	function getExtendedAssetKeywordReplacements()
	{
		return Array();

	}//end getExtendedAssetKeywordReplacements()


	/**
	* Determine if descending sort option was selected
	*
	* Looks at request data before consulting saved attributes
	*
	* @return boolean
	* @access public
	*/
	function isDescending()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_direction';

		$sort_order = array_get_index($_REQUEST, $field_name);
		switch ($sort_order) {
			case '1':
				$result = TRUE;
			break;

			case '0':
				$result = FALSE;
			break;
			default:
				$result = $this->attr('reverse_sort');
		}

		return $result;

	}//end isDescending()


	/**
	* Register a form field name
	*
	* This is useful if we want to determine whether a field has been used in the form
	* This functionality is required for being able to determine if a field was used
	* during output creation in the printContents. This is useful if to avoid field name
	* conflicts or to perform additional logic like using it in javascript, etc
	*
	* @param string	$field_name	Name of the form field
	*
	* @return void
	* @access public
	*/
	function registerFormField($field_name)
	{
		if (!empty($field_name)) {
			$this->_tmp['used_form_fields'][] = $field_name;
		}

	}//end registerFormField()


	/**
	* Return an array of form fields registered for this asset
	*
	* Registered fields are those that have already been used
	* i.e. as a result of a Keyword replacement request
	*
	* @return array
	* @access public
	*/
	function getUsedFormFields()
	{
		return array_get_index($this->_tmp, 'used_form_fields', Array());

	}//end getUsedFormFields()


	/**
	* Gets current sort option
	*
	* Looks at the state to determine which sort option is currently selected
	* Returns one entry from  the 'sort_by' attribute or an empty Array.
	* -
	* -
	* Array(
	*	name => String
	*	type => String
	*	params => Array()
	*  )
	* -
	* params array changes depending on the type value
	* valid types are: field, asset_attrib
	* -
	* if type = "field", params is
	* Array(
	*	field => 'assetid'
	* )
	* where field contains has a name of db column in the asset table to sort by
	* -
	* if type is "asset_attrib" params is
	* Array(
	*	type_code => 'page'
	*	attribute => 'short_name'
	* )
	* where type_code is the type code of the asset whose attribute is being used
	* and 'attribute' is the name of that attribute (as defined by the management class)
	*
	* @return array
	* @access public
	*/
	function getSortInfo()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_by';

		$default_sort_by = $this->attr('default_sort_by');

		$sort_by = array_get_index($_REQUEST, $field_name, $default_sort_by);
		$sort_by_list = $this->attr('sort_by');
		if (!isset($sort_by_list[$sort_by])) {
			$sort_by = $default_sort_by;
		}
		return array_get_index($sort_by_list, $sort_by, Array());

	}//end getSortInfo()


	/**
	* Recursive function to build multi level grouping
	*
	* (actually this is now non-recursive, function name will have to be fixed!)
	* Will construct an array in the following form:
	* if it is a  group its index will be group name (assetid or string) and its
	* contents an array if its an asset in a group its index will be an assetid
	* and its contents will be a typecode
	* The last paramater &$results will hold the contents at the end of the
	* recursive calling instead of working off a return basis
	*
	* @param array	$group_levels	remaining group levels to iterate
	* @param array	$assetids		asset ids that we are working with in this iteration
	* @param array	&$results		reference to the array/section of array we are working on
	* @param array	$group_name_filters	a list of values that must match with group name
	*
	* @return void
	* @access public
	*/
	function groupAssetsRecursively($group_levels, $assetids, &$results, $group_name_filters = Array())
	{
		$min_depth = $this->attr('min_depth');
		if ($min_depth == '') $min_depth = NULL;
		$max_depth = $this->attr('max_depth');
		if ($max_depth == '') $max_depth = NULL;

		$root_nodes = $this->getRootNodes(FALSE);

		// we need assets and groups, so if none given, let's bail
		if (empty($assetids) || empty($group_levels)) {
			return;
		}

		// we need to reverse this because we want to do the inner level first
		$group_levels = array_reverse($group_levels);

		$asset_groups = Array();
		foreach ($assetids as $assetid => $group_datas) {
			$asset_groups[] = Array('assetid' => $assetid);
		}

		// keep track of the last time we encountered a parent asset group
		$last_parent_asset = FALSE;
		$limit_array = Array();

		foreach ($group_levels as $group_key => $group_level) {
			array_unshift($limit_array, array_get_index($group_level, 'max_children', NULL));
			$new_asset_groups = Array();

			// go one node UP the tree, no matter whether we're going up or down
			if (!is_null($min_depth)) {
				$min_depth -= 1 * ($this->attr('direction') == 'up' ? -1 : 1);
			}
			if (!is_null($max_depth)) {
				$max_depth -= 1 * ($this->attr('direction') == 'up' ? -1 : 1);
			}

			foreach ($asset_groups as $key => $group_data) {
				$assetid = $group_data['assetid'];

				if ($last_parent_asset === FALSE) {
					$current_parent = $assetid;
				} else {
					$current_parent = $group_data['group'][$last_parent_asset];
				}
				switch ($group_level['group_type']) {
					case 'parent_asset':
						$restrict_types = array_get_index($group_level, 'restrict_types', Array());
						$direct_parent_only = array_get_index($group_level, 'direct_parent_only', FALSE);

						// If our direct parents have been cached from a Listing
						// Engine-type asset (eg. Asset Listing does this if there
						// is only one level and it's a direct parent grouping),
						// then use that instead of working it out each time
						if ($direct_parent_only && isset($this->_tmp['direct_parent_grouping'][$current_parent])) {
							$value = $this->_tmp['direct_parent_grouping'][$current_parent];
						} else {
							$value = $this->getGroupableParentAssetids($current_parent, $restrict_types, $direct_parent_only);
						}

						// skip this whole section if both depth fields are NULL
						// otherwise, we are limiting by parent's depth, relative
						// to what is set for leaf nodes
						if (!is_null($min_depth) || !is_null($max_depth)) {

							if (!isset($this->_tmp['grouping_treeids'])) {
								$this->_tmp['grouping_treeids'] = array();
							}

							foreach ($value as $key => $parent_assetid) {

								/**
								* If we haven't grabbed the assetid's from the tree before,
								* do it - then cache the results.
								* Saves hitting the db for the same info for each asset
								* that we're grouping
								*/
								if (!isset($this->_tmp['grouping_treeids'][$parent_assetid])) {
									if ($this->attr('direction') == 'down') {

										$this->_tmp['grouping_treeids'][$parent_assetid] = $GLOBALS['SQ_SYSTEM']->am->getParents($parent_assetid, '', FALSE, NULL, NULL, TRUE, $min_depth, $max_depth);
									} else {
										$this->_tmp['grouping_treeids'][$parent_assetid] = $GLOBALS['SQ_SYSTEM']->am->getChildren($parent_assetid, '', FALSE, NULL, NULL, NULL, TRUE, $min_depth, $max_depth);
									}
								}
								$parents = $this->_tmp['grouping_treeids'][$parent_assetid];
								$intersection = array_intersect(array_keys($parents), $root_nodes);
								if (empty($intersection) && !in_array($parent_assetid, $root_nodes)) unset($value[$key]);

							}
						}

					break;

					case 'metadata':
						$metadata_field = NULL;
						$metadate_field_id = NULL;
						if (!empty($group_level['metadata_field'])) {
							$metadata_field_ids = $group_level['metadata_field'];
							if(is_scalar($metadata_field_ids)) $metadata_field_ids = Array($metadata_field_ids);
						}

						$presentation_value = FALSE;
						if (!empty($group_level['metadata_sort_type'])) {
							if ($group_level['metadata_sort_type'] === 'presentation') {
								$presentation_value = TRUE;
							}
						}
						// get the metadata manager if we haven't already
						if (!isset($mm)) {
							$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						}
						$value = Array();
						foreach($metadata_field_ids as $metadata_field_id) {
							$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field_id);
							$field_type = $metadata_field->type();
							if(empty($metadata_field)) continue;
							$new_value = $mm->getMetadataValueByAssetid($current_parent, $metadata_field_id, TRUE, $presentation_value);

							// port the feature added #4039, although not perfectly sensible but still have to keep it
							// if using raw value, always use the value of select field, because the value is always dispalyed in backend interface
							// if using presentation value, that's whatever is displayed in frontend (key or value depends on schema setting)
							if (($field_type === 'metadata_field_hierarchy' || $field_type === 'metadata_field_select') && !$presentation_value)  {
							    $new_value= preg_replace('/\\\;/', ';',  $metadata_field->getValueFromKey($new_value));
							}

							// if it's multi-options field, we need to extract individual option
							$individual_entry_grouping = array_get_index($group_level,'group_by_individual',  TRUE);
							if($individual_entry_grouping) {
							    switch($field_type) {
								case 'metadata_field_multiple_text':
								case 'metadata_field_hierarchy':
								case 'metadata_field_select':
								    $new_value = explode('; ', $new_value);
								    break;
								case 'metadata_field_thesaurus':
								    $new_value = explode(',', $new_value);
								    break;
							    }
							}

							if(!is_array($new_value)) {
							    $value[] = $new_value;
							}
							else {
							    $value = array_merge($value, $new_value);
							}
							if(empty($value)) $value = '';
						}
					break;

					case 'standard_asset':
						if (!isset($this->_tmp['assets_info'][$current_parent])) {
							$this->_tmp['assets_info'][$current_parent] = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_parent));
						}
						if (!empty($this->_tmp['assets_info'][$current_parent])) {
							$asset_info = $this->_tmp['assets_info'][$current_parent];

							$standard_asset_field = $group_level['field'];
							switch ($standard_asset_field) {
								case 'assetid':
									$value = $current_parent;
								break;

								case 'status':
									$value = get_status_description($asset_info['status']);
								break;

								default:
									$value = $asset_info[$standard_asset_field];
								break;
							}
						}
					break;

					case 'keyword':
							$keyword = $group_level['keyword'];
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_parent);
							if(!is_null($asset)) {
								$value = $asset->getKeywordReplacement($keyword);
							}
							else {
								$value = '';
							}
					break;

					case 'attribute':
						// implementation reserved
					break;
				}//end switch


				if(!is_array($value)) $value = Array($value);

				foreach ($value as $sub_value) {
					// should we allow empty group name?
					$allow_blank_group_name = array_get_index($group_level, 'group_by_blank', TRUE);
					if(!$allow_blank_group_name && empty($sub_value)) continue;

					// should we allow group name not matching search query?
					$group_name_match_search_query = array_get_index($group_level, 'group_name_match_query', FALSE);
					if($group_name_match_search_query) {
					    $is_filter = TRUE;
					    foreach ($group_name_filters as $filter) {
						    if(empty($filter)) continue;
						    if(!is_array($filter)) $filter = Array($filter);
						    foreach($filter as $part) {
							if(is_scalar($part) && strpos($sub_value, $part) !== FALSE)
								$is_filter = FALSE;
						    }
					    }
					    if($is_filter) continue;
					}


					$group_data['group'][$group_key] = $sub_value;
					$new_asset_groups[] = $group_data;
				}
			}//end foreach on asset groups

			$asset_groups = $new_asset_groups;

			if ($group_level['group_type'] == 'parent_asset') {
				$last_parent_asset = $group_key;
			}

		}//end foreach on group level

		$results = Array();

		$group_keys = array_keys($group_levels);

		// we need to flip the keys back to what they were to construct the array
		foreach ($asset_groups as $group_data) {
			$base =& $results;
			foreach (array_reverse($group_keys) as $group_key) {
				$key = $group_data['group'][$group_key];
				// Postgres returns FALSE when no records are found
				if (is_null($key) || (MatrixDAL::getDbType() == 'pgsql' && $key === FALSE)) $key = '';
				$base =& $base[$key];
			}
			$base[$group_data['assetid']] = $assetids[$group_data['assetid']];

		}

		$this->limitAssetsRecursively($results, $limit_array);

		unset($this->_tmp['root_node_treeids']); // just in case we are nested in a different context later

	}//end groupAssetsRecursively()


	/**
	* Recursive function to limit the number of assets in each group
	*
	* The second parameter, $limit_array, contains a list of integers that determine
	* how many assets/child groups will be kept at each level. eg. in a listing
	* engine-derived asset with two groups, a $limit_array of Array(2,3) means that
	* the top-level group will contain at most 2 child groups, and each child group
	* will contain at most 3 child assets. Any value can be NULL or zero for all
	* assets to be kept. The $limit_array passed to each recursive level is reduced
	* by one (the top level) each time.
	*
	* @param array	&$results		The grouped and sorted results list (array of mixed)
	* @param array	$limit_array	Contains group limits for each level (array of int|null)
	*
	* @return void
	* @access public
	*/
	function limitAssetsRecursively(&$results, $limit_array)
	{
		$num = array_shift($limit_array);

		foreach ($results as $key => $val) {
			if (!is_null($num) && ($num >= 1)) {
				while (count($results[$key]) > $num) {
					array_pop($results[$key]);
				}
			}

			if (!empty($limit_array)) {
				$this->limitAssetsRecursively($results[$key], $limit_array);
			}
		}

	}//end limitAssetsRecursively()


	/**
	* Get all the parents of this asset which can be used for grouping.
	*
	* If we are grouping by a type that is selected as a proxy parent, then we
	* only want to group by those parents of that type which are actually acting
	* as proxies.
	*
	* @param int		$child_assetid		the asset to find the parents of
	* @param array		$restrict_types		Group type restrictions
	* @param boolean	$direct_parent_only	include the direct parent which matches the restriction type
	*
	* @return array
	* @access public
	*/
	function getGroupableParentAssetids($child_assetid, $restrict_types=Array(), $direct_parent_only=FALSE)
	{
		// if there are proxy asset types selected and results are grouped by one of those asset types, return the parent
		// asset ids which are cached in the convertProxyAssetTypes() method.
		$proxy_settings = $this->attr('proxy_types');
		if (!empty($proxy_settings)) {
			if (!empty($this->_tmp['proxy_grouping'][$child_assetid])) {
				foreach ($this->_tmp['proxy_grouping'][$child_assetid] as $parent_type_code => $parent_assetids) {
					// if the type_code of the parent assets is one of the grouped type_code, return those parent ids
					if (in_array($parent_type_code, $restrict_types['type_code'])) {
						return array_values(array_unique($parent_assetids));
					}
				}
			}
		}

		$parent_link_assetids = Array();

		$root_nodes = $this->getRootNodes(FALSE);
		if (empty($root_nodes) || (strpos($child_assetid, ':') !== FALSE) || ($this->attr('direction') == 'up')) {
			$parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($child_assetid, $this->attr('link_types'), '', TRUE, 'minor');
			$parent_link_assetids = Array();
			foreach ($parent_links as $parent_link) {
				$parent_link_assetids[] = $parent_link['majorid'];
			}
		} else {
			$parent_link_assetids = $this->_getGroupableParentAssetids($child_assetid, $restrict_types, $root_nodes, $direct_parent_only);
		}
		// help the malfunctioning bridge a bit
		if (empty($parent_link_assetids) && strpos($child_assetid, ':') !== FALSE && $direct_parent_only) {
			$id_parts = explode(':', $child_assetid);
			$parent_link_assetids[] = $id_parts[0];
		}

		return $parent_link_assetids;

	}//end getGroupableParentAssetids()


	/**
	* Takes an array of results and modifies it according to the rules set on
	* the Proxy Asset Types screen. Any asset set as a proxy is replaced with
	* its children of specified type_code. The children inherit the score from
	* the proxy
	*
	* @param array	&$results	Proxy Types
	*
	* @return array
	* @access private
	*/
	function &convertProxyAssetTypes(&$results)
	{
		if (empty($results)) return $results;

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$proxy_settings = $this->attr('proxy_types');
		if (empty($proxy_settings)) {
			// We should be able to return the results array straight back, but
			// we've found sometimes it returns an empty array (PHP 4.3 ref bug??).
			// So dump it off to another variable by value and return that
			$new_results = $results;
			return $new_results;
		}

		$collated_types = Array();
		$result_types = $am->getAssetInfo(array_keys($results), 'asset', FALSE, 'type_code');
		foreach ($result_types as $result_assetid => $result_type) {
			$collated_types[$result_type][] = $result_assetid;
		}

		$new_results = Array();
		$this->_tmp['proxy_grouping'] = Array();
		foreach ($collated_types as $result_type => $result_assetids) {
			if (isset($proxy_settings[$result_type])) {
				if (empty($proxy_settings[$result_type])) {
					return $results;
				}

				$converted_types = Array();
				foreach ($proxy_settings[$result_type] as $type_code => $inherit) {
					if ($inherit) {
						$converted_types = array_merge($am->getTypeDescendants($type_code, TRUE), $converted_types);
					} else {
						$converted_types[] = $type_code;
					}
				}

				$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($result_assetids, SQ_SC_LINK_SIGNIFICANT, $converted_types);

				foreach ($children as $orig_assetid => $links) {
					foreach ($links as $link) {
						// keep the original score
						$new_results[$link['minorid']] = $results[$orig_assetid];
						// store the temporary array of child => parents for use later in getGroupableParentAssetids() method
						// if neccessary. The structure of the array is
						// Array(childid => Array(parent_type_code => Array(parentid))) - A child asset can have many parents in
						// different type codes. Grouping parentids like this will make it easier to get them all later
						$this->_tmp['proxy_grouping'][$link['minorid']][$result_type][] = $orig_assetid;
					}
				}

				// filterAssetStatuses is overridden in search_page, so call it directly
				Listing_Engine::filterAssetStatuses($new_results);
			}
		}

		return $new_results;

	}//end convertProxyAssetTypes()


	/**
	* Get all the parents of this asset which can be used for grouping.
	*
	* @param array		$child_assetid		the asset to find the parents of
	* @param array		$restrict_types		Group type restrictions
	* @param array		$root_nodes			Root Nodes for this listing
	* @param boolean	$direct_parent_only	include the direct parent which matches the restriction type
	*
	* @return array
	* @access public
	*/
	function _getGroupableParentAssetids($child_assetid, $restrict_types=Array(), $root_nodes=Array(), $direct_parent_only=FALSE)
	{
		// get single treeid for each root node, children of root node should be the same on all trees
		$groupable_parents = Array();
		if (!isset($this->_tmp['root_node_treeids'])) {
			foreach ($GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($root_nodes) as $aid => $treeids) {
				$this->_tmp['root_node_treeids'][$aid] = reset($treeids);
			}
		}
		$lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($child_assetid, 0, $this->_tmp['root_node_treeids'], 'type_code');

		foreach ($lineages as $lineage_info) {

			// only get direct parent, if it is set in the group by option
			if ($direct_parent_only && !empty($lineage_info['lineage'])) {
				$all_assetids = array_keys($lineage_info['lineage']);
				$direct_parent_assetid = array_pop($all_assetids);
				$direct_parent_type = array_pop($lineage_info['lineage']);
				// in the form of (assetid => type_code)
				$lineage_info['lineage'] = Array($direct_parent_assetid => $direct_parent_type);
			}

			// if not one of the selected link types continue
			if (!($lineage_info['link_type'] & $this->attr('link_types'))) {
				continue;
			}

			// if no types selected then group by root nodes
			if (!isset($restrict_types['type_code']) || empty($restrict_types['type_code'])) {
				$groupable_parents[] = array_shift(array_keys($lineage_info['lineage']));
				continue;
			}
			// start from the end of the current lineage
			$lineage = array_reverse($lineage_info['lineage'], TRUE);
			$add = FALSE;

			foreach ($lineage as $parentid => $type_code) {
				if (in_array($type_code, $restrict_types['type_code'])) {
					$add = TRUE;
				} else {
					$inherit_vals = $restrict_types['inherit'];
					foreach ($restrict_types['type_code'] as $type) {
						$inherit = array_shift($inherit_vals);
						if (!isset($this->_tmp['type_decendants'][$type])) {
							$this->_tmp['type_decendants'][$type] = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type, TRUE);
						}
						if ($inherit && in_array($type_code, $this->_tmp['type_decendants'][$type])) {
							$add = TRUE;
							break;
						}
					}
				}

				if ($add) {
					// we have found the parent for this lineage
					$groupable_parents[] = $parentid;
					break;
				}
			}
		}//end foreach

		return array_unique($groupable_parents);

	}//end _getGroupableParentAssetids()


	/**
	* Recursive function to sort the group keys
	*
	* Returns void but returns a modified array (in sort order only, contents
	* themselves are unaffected) in &$results
	*
	* @param array	$group_levels	remaining group levels to iterate
	* @param array	$assetids		asset ids that we are working with in this
	*								iteration
	* @param array	&$results		reference to the array/section of array we are
	*								working on
	*
	* @return void
	* @access public
	*/
	function sortGroups($group_levels, $assetids, &$results)
	{
		// we need assets, so if none given, let's bail
		if (empty($assetids)) return;

		// point us at the first element, discarding it as we go
		$group_type = array_shift($group_levels);

		// sort the groups
		switch ($group_type['group_type']) {
			case 'parent_asset':
				// find the info on the assets that are being used, sort by them,
				// then re-order the results based upon them
				if (!empty($group_type['sorting_metadata_field'])) {
					$sorting_metadata_field_id = $group_type['sorting_metadata_field'];
					if (!empty($sorting_metadata_field_id)) {
						$sorting_metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($sorting_metadata_field_id);
						$field_type = $sorting_metadata_field->type();
					}
					if (!is_null($sorting_metadata_field)) {
						// get the metadata manager if we haven't already
						if (!isset($mm)) {
							$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						}//end if
						$group_order = Array();

						$presentation_value = FALSE;
						if (!empty($group_type['metadata_sort_type'])) {
							if ($group_type['metadata_sort_type'] === 'presentation') {
								$presentation_value = TRUE;
							}
						}

						foreach ($results as $key => $arrayInfo) {
							$group_order[$key] = $mm->getMetadataValueByAssetid($key, $sorting_metadata_field_id, TRUE, $presentation_value);
							// use value of selection if raw display is required
							if (($field_type === 'metadata_field_hierarchy' || $field_type === 'metadata_field_select') && !$presentation_value)  {
							    $group_order[$key] = preg_replace('/\\\;/', ';',  $sorting_metadata_field->getValueFromKey($group_order[$key]));
							}
						}//end foreach
					}//end if
				} else {
					$group_order = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($results), 'asset', FALSE,array_get_index($group_type, 'sort_by', 'assetid'));

				}//end else

				if (array_get_index($group_type, 'sort_order', 'asc') == 'asc') {
					asort($group_order);
				} else {
					arsort($group_order);
				}
				$group_results = Array();
				foreach (array_keys($group_order) as $assetid) {
					$group_results[$assetid] =& $results[$assetid];
				}
				$results = $group_results;

			break;

			case 'metadata':
			case 'standard_asset':
			case 'keyword':
				// metadata and standard asset field groups are sorted just on
				// the key's value, because the key IS the value
				if (array_get_index($group_type, 'sort_order', 'asc') == 'asc') {
					ksort($results);
				} else {
					krsort($results);
				}
			break;

		}//end switch

		// if their are group levels remaining, we want to call
		// this function again for each of the groups we have made
		if (count($group_levels) > 0) {
			foreach ($results as $index => $group) {
				$this->sortGroups($group_levels, $group, $results[$index]);
			}
		}

	}//end sortGroups()


	/**
	* Reindexes the group format bodycopies so any gaps are closed and bodycopy
	* names are referring to their correct group numbers
	*
	* @return boolean
	* @access public
	*/
	function reindexGroupFormats()
	{
		// if we have no group formats, it's not worth our time
		$group_folder = $this->getFolder('group_formats');
		$groups = $this->attr('asset_grouping');
		if (empty($groups)) return TRUE;


		// set forced run level because we want to change the names of these
		// formats whether we have permission or not
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		$i = 1;
		foreach ($groups as $group) {
			// update the group format
			$format_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($group['format_assetid']);
			if ($format_asset) {
				$format_asset->setAttrValue('name', 'Group Level '.$i.' Format');
				$format_asset->saveAttributes();
			}

			$i++;
		}

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end reindexGroupFormats()


	/**
	* Preprocess the Asset Selection values before displaying the page or redirecting
	* Overwritten by the children
	*
	* @return boolean
	* @access public
	*/
	function processAssetSelections()
	{
		return TRUE;

	}//end processAssetSelections()


	/**
	* Provide the default values for each of the selection fields
	* Overwritten by the children
	*
	* @param string	$selection_name	name
	* @param string	$assetid		asset id
	*
	* @return int
	* @access public
	*/
	function getAssetSelectionValue($selection_name, $assetid)
	{
		return 0;

	}//end getAssetSelectionValue()


	/**
	* List the default settings format for Asset Selections
	*
	* @return array
	* @access public
	*/
	function getAssetSelectionDefaults()
	{
		return Array(
				'type'		=> 'radio',
				'options'	=> Array(
								'allow_multiple'	=> 0,
								'allow_negative'	=> 0,
								'restrict_range'	=> Array(
														'enable'	=> 0,
														'min'		=> 0,
														'max'		=> 10,
													   ),
							   ),
				'permanent'	=> 0,
			   );

	}//end getAssetSelectionDefaults()


	/**
	* List the default settings format for Asset Targets
	*
	* @return array
	* @access private
	*/
	function getAssetTargetDefaults()
	{
		return Array(
				'label'			=> '',
				'target_asset'	=> Array(
									'assetid'	=> 0,
									'url'		=> '',
								   ),
				'target_url'	=> '',
				'permanent'		=> 0,
			   );

	}//end getAssetTargetDefaults()


	/**
	* Recursively count array elements
	*
	* @param array	&$array	Array to count
	* @param string	$limit	Number of levels to traverse
	*
	* @return int
	* @access private
	*/
	function _arrayCountRecursive(&$array, $limit=NULL)
	{
		$count = 0;

		for (reset($array); NULL !== ($k = key($array)); next($array)) {
			$el =& $array[$k];
			if (is_array($el) && (is_null($limit) || ($limit > 0))) {
				$count += $this->_arrayCountRecursive($el, (is_null($limit) ? NULL : $limit - 1));
			} else {
				$count++;
			}
		}

		return $count;

	}//end _arrayCountRecursive()


	/**
	* Return a list of position formats
	*
	* @return array
	* @access private
	*/
	function _getPositionFormats()
	{
		if (!isset($this->_tmp['position_formats'])) {
			$this->_tmp['position_formats'] = Array();

			$position_folder = $this->getFolder('position_formats');
			$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($position_folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE);
			foreach ($format_links as $link_data) {
				$position = substr($link_data['value'], 9);
				$this->_tmp['position_formats'][$position] = $link_data['minorid'];
			}
		}

		return $this->_tmp['position_formats'];

	}//end _getPositionFormats()


//--		KEYWORD DESCRIPTION		   --//


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$parent_folders = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'folder', TRUE);

		// if we're being handled here (instead of one of our extensions),
		// it must be from one of our format bodycopies, or something else altogether,
		// so we'll try to find out which of the format types it is.
		$group_folder = $this->getFolder('group_formats');
		$position_folder = $this->getFolder('position_formats');
		$type_folder = $this->getFolder('type_formats');

		$keywords = Array();
		if (!is_null($group_folder) && in_array($group_folder->id, array_keys($parent_folders))) {
			// group format
			$keywords = $this->_getGroupFolderKeywords();

		} else if (!is_null($position_folder) && in_array($position_folder->id, array_keys($parent_folders))) {
			// position format
			$keywords = $this->_getGenericSingleAssetFormatKeywords();

		} else if (!is_null($type_folder) && in_array($type_folder->id, array_keys($parent_folders))) {
			// specific or default type format
			$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($type_folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			$type_codes = Array();
			foreach ($type_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$type_codes[] = $link_info['value'];
				}
			}

			$type_name = NULL;
			if ((count($type_codes) == 1) && ($type_codes[0] != 'default_format')) {
				$type_name = $type_codes[0];
			}
			$keywords = $this->_getGenericSingleAssetFormatKeywords($type_name);

		} else {
			// page contents format
			$keywords = $this->_getContentsKeywords();
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Get keywords from another asset
	*
	* Gets a list of keywords either from a base asset or a specified asset type
	*
	* @param string	$type	Asset type to get keywords from
	*
	* @return array
	* @access private
	*/
	function _getGenericSingleAssetFormatKeywords($type=NULL)
	{
		$keywords = $this->_getAdditionalSingleAssetFormatKeywords();

		if (!empty($type)) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
			$dummy_asset = new $type();
			$keywords += $dummy_asset->getAvailableKeywords();
			unset($dummy_asset);
		} else {
			$keywords += Asset::getAvailableKeywords();
		}

		return $keywords;

	}//end _getGenericSingleAssetFormatKeywords()


	/**
	* Get additional keywords to be used in the single-asset format
	*
	* Provides keywords to be used alongside those provided by assets themselves.
	* These keywords are understood only by this asset
	*
	* @return void
	* @access private
	*/
	function _getAdditionalSingleAssetFormatKeywords()
	{
		$keywords['Asset Lineage'] = translate('Asset Lineage');

		$keywords['Asset Lineage Linked'] = translate('Asset Lineage Linked');

		$keywords['asset_contents'] = translate('Asset Contents (including paintlayout)');

		$keywords['asset_contents_paint_'] = translate('Asset Contents (user-defined paint layout)');

		$keywords['asset_name_linked'] = translate('Asset Name Linked');

		$keywords['asset_short_name_linked'] = translate('Short Asset Name Linked');

		$keywords['asset_position'] = translate('Asset Position');

		$keywords['asset_odd_even'] = translate('Asset\'s position is odd or even');

		$keywords['root_nodes'] = translate('Root Nodes (separated by commas)');

		$keywords['asset_contents_raw'] = translate('Asset Contents (excluding paintlayout)');


		// asset selections
		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				$keywords[$name.'_asset_selection'] = sprintf(translate('Asset Selection (%s)'), $name);

			}
		}

		return $keywords;

	}//end _getAdditionalSingleAssetFormatKeywords()


	/**
	* Return the available keywords from the parents
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*	  'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();
		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*	  'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function _getContentsKeywords()
	{
		$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings('page_asset_listing');

		$keywords['sort_by']				= translate('Sort By');

		$keywords['sort_order']				= translate('Sort Order');

		$keywords['reset_button']			= translate('Reset Button');

		$keywords['submit_button']			= translate('Submit Button');

		$keywords['root_nodes']				= translate('Root Nodes (separated by commas)');


		$keywords['asset_listing']			= translate('Asset Listing');

		$keywords['previous_page']			= translate('Previous Page Link');

		$keywords['previous_page_href']		= translate('Previous page Href');

		$keywords['next_page']				= translate('Next Page Link');

		$keywords['next_page_href']			= translate('Next Page Href');


		$keywords['page_list']				= translate('Page List');

		if ($this->attr('group_by') == 'letter') {
			$keywords['page_list_without_unused'] = translate('Page List (without unused letters)');

		}
		$keywords['page_list_X']			= translate('Sliding page list showing X page numbers at a time');


		$keywords['page_number']			= translate('Page Number');

		$keywords['asset_count']			= translate('Total Assets Listed');

		$keywords['page_asset_count']		= translate('Assets Listed On This Page');

		$keywords['total_pages']			= translate('Total Pages');

		$keywords['first_asset_position']	= translate('First Asset on Page\'s Position Number');

		$keywords['last_asset_position']	= translate('Last Asset on Page\'s Position Number');

		$keywords['unique_asset_count']		= translate('Number of unique assets');

		$keywords['structured_root_node']	= translate('Structured Root Node Selector');


		$keywords['select_all_js_code']		= 'Select All Script Code';

		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				if (array_get_index($settings, 'type', '') == 'radio' && !empty($settings['options']['allow_multiple'])) {
					$keywords[$name.'_check_all'] = sprintf(translate('Asset Selection Check All (%s)'), $name);

				}
			}
		}

		$targets = $this->attr('asset_targets');
		if (is_array($targets) && !empty($targets)) {
			foreach ($targets as $name => $settings) {
				$keywords[$name.'_asset_target'] = sprintf(translate('Asset Target Button (%s)'), $name);

			}
		}

		return $keywords;

	}//end _getContentsKeywords()


	/**
	* Get the keywords which are specific in Type Formats
	*
	* Marked deprecated on 15/09/2005
	*
	* @deprecated
	* @return array
	* @access protected
	*/
	function _getTypeFormatsKeywords()
	{
		$keywords['Asset Lineage'] = translate('Asset Lineage');

		$keywords['Asset Lineage Linked'] = translate('Asset Lineage Linked');

		$keywords['asset_contents'] = translate('Asset Contents (including paintlayout)');

		$keywords['asset_name_linked'] = translate('Asset Name Linked');

		$keywords['asset_short_name_linked'] = translate('Short Asset Name Linked');

		$keywords['asset_position'] = translate('Asset Position');

		$keywords['root_nodes'] = translate('Root Nodes (separated by commas)');



		// asset selections
		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				$keywords[$name.'_asset_selection'] = sprintf(translate('Asset Selection (%s)'), $name);

			}
		}

		return $keywords;

	}//end _getTypeFormatsKeywords()


	/**
	* Get the keywords which are specific in Group Folder
	*
	* @return array
	* @access protected
	*/
	function _getGroupFolderKeywords()
	{
		$fake_asset = new Asset();
		$keywords = $fake_asset->getAvailableKeywords();
		foreach ($keywords as $key =>$keyword) {
			$key1 = substr_replace($key, 'parent', 0, 5);
			$keywords[$key1] = $keywords[$key];
			unset($keywords[$key]);
		}
		$keywords['group_listing']				   = translate('Asset Group Listing');

		$keywords['group_name']					   = translate('Asset Group Name');

		$keywords['group_name_linked']			   = translate('Asset Group Name Linked');

		$keywords['parent_contents']			   = translate('Contents of the Parent Asset');

		$keywords['parent_contents_raw']		   = translate('Raw Contents of the Parent Asset');

		$keywords['group_immed_child_group_count'] = translate('Asset Group Immediate Child Group Count');

		$keywords['group_total_child_asset_count'] = translate('Asset Group Total Child Asset Count');


		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				if (array_get_index($settings, 'type', '') == 'radio' && !empty($settings['options']['allow_multiple'])) {
					$keywords[$name.'_check_all_in_group'] = sprintf(translate('Asset Selection Check All In Group (%s)'), $name);

				}
			}
		}
		return $keywords;

	}//end _getGroupFolderKeywords()


//--		KEYWORD REPLACEMENT		   --//


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	* -
	* Overridden in listing_engine to handle dynamic keywords in the page
	* contents, such as Asset Targets
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($original_keyword)
	{
		if (empty($original_keyword)) return '';
		$keyword = parse_keyword($original_keyword, $modifiers);

		// check the keyword against the dynamic keywords
		// asset targets
		if (preg_match('/^(.*)(_asset_target)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2])) {
				$args		  = $matches[1];
				$keyword_stem = $matches[2];
			}
		}

		// setup the replacement function call
		if (!empty($keyword_stem) && !empty($args)) {
			$func_name = 'get'.ucwords_no_space($keyword_stem).'KeywordReplacement';
			if (method_exists($this, $func_name)) {
				$replacement = $this->$func_name($args);
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
				return $replacement;
			}
		}
		// otherwise, fall back to the parent
		$replacement = parent::getKeywordReplacement($keyword);

		if ($replacement == '%'.$keyword.'%' && empty($modifiers)) {
			return $replacement;
		} else if($replacement == '%'.$keyword.'%' && !empty($modifiers)) {
			return '%'.$original_keyword.'%';
		}

		if (!is_null($replacement)) {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		}
		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Return a subset of the replacements for the keywords available to the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*	  'keyword' => 'name',
	* )
	* </pre>
	*
	* @param array	$keywords	the keywords from the content
	*
	* @return array
	* @access public
	*/
	function getContentsKeywordReplacements($keywords=Array())
	{
		$replacements = Array();

		$selections = $this->attr('asset_selections');
		$selection_defaults = $this->getAssetSelectionDefaults();

		foreach ($selections as $selection_name => $settings) {

			if (!in_array($selection_name.'_check_all', $keywords)) {
				continue;
			}
			$this->registerFormField($selection_name.'_check_all');

			// predefine some commonly used settings, for brevity's sake
			$selection_options = array_get_index($settings, 'options', $selection_defaults['options']);
			$selection_allow_multiple = array_get_index($selection_options, 'allow_multiple', $selection_defaults['options']['allow_multiple']);

			// only print the 'check all' checkbox if we're only dealing with checkboxes
			if ((array_get_index($settings, 'type', $selection_defaults['type']) == 'radio') && $selection_allow_multiple) {
				ob_start();
				$onclick = Array();
				$group_cb_id  = 'all';
				$control_name = make_valid_html_id($this->_getSelectionFieldNamePrefix($selection_name).str_replace(' ', '_', $group_cb_id));
				$this->registerFormField($control_name);
				check_box($control_name, '1', FALSE, "sq_listing_check_state(this, '".$this->_getSelectionFieldNamePrefix($selection_name)."')");
				$replacements[$selection_name.'_check_all'] = ob_get_clean();
				$replacements[$selection_name.'_selection_id'] = $control_name;
			} else {
				$replacements[$selection_name.'_check_all'] = '';
			}
		}

		return $replacements;

	}//end getContentsKeywordReplacements()


	/**
	* Get the replacement for "sort_by" keyword
	*
	* Produces a dropdown list of all the sorting options registered for this asset
	*
	* @return string
	* @access public
	*/
	function getSortByKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_by';
		$this->registerFormField($field_name);

		$default_sort_by = $this->attr('default_sort_by');
		$sort_by = array_get_index($_REQUEST, $field_name, $default_sort_by);

		$sort_by_list = $this->attr('sort_by');
		if (!isset($sort_by_list[$sort_by])) {
			$sort_by = $default_sort_by;
		}

		$options = Array();
		foreach ($sort_by_list as $key => $value) {
			$options[$key] = $value['name'];
		}

		ob_start();
			combo_box($field_name, $options, FALSE, $sort_by);
		return ob_get_clean();

	}//end getSortByKeywordReplacement()


	/**
	* Get the replacement for "sort_order" keyword
	*
	* @return string
	* @access public
	*/
	function getSortOrderKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_direction';
		$this->registerFormField($field_name);

		$options = Array(
					'0'	=> $this->attr('sort_direction_asc_text'),
					'1'	=> $this->attr('sort_direction_desc_text'),
				   );

		ob_start();
			combo_box($field_name, $options, FALSE, (int)$this->isDescending());
		return ob_get_clean();

	}//end getSortOrderKeywordReplacement()


	/**
	* Get the replacement for "submit_button" keyword
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_submit_button';
		$this->registerFormField($field_name);

		$button_text = $this->attr('submit_button_text');

		ob_start();
			submit_button($field_name, $button_text);
		return ob_get_clean();

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Get the replacement for "reset_button" keyword
	*
	* @return string
	* @access public
	*/
	function getResetButtonKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_reset_button';
		$this->registerFormField($field_name);

		$button_text = $this->attr('reset_button_text');

		ob_start();
			reset_button($field_name, $button_text);
		return ob_get_clean();

	}//end getResetButtonKeywordReplacement()


	/**
	* Get the replacement for "root_nodes" keyword
	*
	* @return string
	* @access public
	*/
	function getRootNodesKeywordReplacement()
	{
		if (SQ_IN_BACKEND || SQ_IN_LIMBO) {
			$root_nodes = $this->getRootNodes(TRUE);
		} else {
			$root_nodes = $this->getRootNodes(FALSE);
		}//end if
		return implode(',', $root_nodes);

	}//end getRootNodesKeywordReplacement()


	/**
	* Get the replacements for the dynamic "_asset_target" keyword
	*
	* @param string	$name	name
	*
	* @return string
	* @access public
	*/
	function getAssetTargetKeywordReplacement($name)
	{
		$targets	= $this->attr('asset_targets');
		$prefix		= $this->getPrefix();
		$field_name = $prefix.'_'.$name.'_asset_target';
		$this->registerFormField($field_name);

		if (isset($targets[$name])) {
			$defaults		  = $this->getAssetTargetDefaults();
			$button_text	  = array_get_index($targets[$name], 'label',		 $defaults['label']);
			$target = NULL;

			// determine whether we're using the url or the asset (url takes precedence)
			$target_url		  = array_get_index($targets[$name], 'target_url',	 $defaults['target_url']);
			if ($target_url) {
				$target = $target_url;
			} else {
				$target_asset	  = array_get_index($targets[$name], 'target_asset', $defaults['target_asset']);
				$target_assetid   = array_get_index($target_asset,	 'assetid',		 $defaults['target_asset']['assetid']);
				if ($GLOBALS['SQ_SYSTEM']->am->assetExists($target_assetid)) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($target_assetid);
					$target = $asset->getURL();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($target_assetid);
				}
			}

			if (!is_null($target)) {
				ob_start();
					submit_button($field_name, $button_text, "document.getElementById('".$this->getPrefix()."').action = '".$target."';");
				return ob_get_clean();
			}
		}

		return '';

	}//end getAssetTargetKeywordReplacement()


	/**
	* Get the replacement for "structured_root_node" keyword
	*
	* @return string
	* @access public
	*/
	function getStructuredRootNodeKeywordReplacement()
	{
		$options = $this->attr('structured_dropdown_options');
		if (empty($options['root_node'])) return '';

		if (!empty($options['name'])) {
			$this->registerFormField($options['name']);
		}

		// initial type_codes, before we check for inheritance
		$pass_codes = array_keys($options['asset_types']);
		foreach ($options['asset_types'] as $code => $inherit) {
			if ($inherit == 1) {
				$pass_codes += $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($code);
			}
		}
		$selected = (isset($_REQUEST[$options['name']])) ? Array($_REQUEST[$options['name']]) : Array();

		if (isset($selected[0]) && is_array($selected[0])) {
			$selected = $selected[0];
		}

		ob_start();
			structured_drop_down($options['name'], $options['root_node'], $pass_codes, $selected, $options['width'], $options['height'], $options['max_depth'], $this->attr('check_boxes') == 2 ? TRUE : FALSE, $options['seperator'], TRUE, $options['all_text'], $this->attr('check_boxes'), Array(), TRUE, Array(), FALSE, NULL, $this->attr('exclude_root_nodes'));
			$replacement = ob_get_contents();
		ob_end_clean();
		return $replacement;

	}//end getStructuredRootNodeKeywordReplacement()


	/**
	* Get the replacement for "select_all_js_code" keyword
	*
	* @return string
	* @access public
	*/
	function getSelectAllJSCodeKeywordReplacement()
	{
		return array_get_index($this->_tmp, 'js_relation_code', '');

	}//end getSelectAllJSCodeKeywordReplacement()


//--		ASSET SELECTION HELPERS		   --//


	/**
	* Analyse "check-all" relations recursively
	*
	* @param array	&$todo		assets to list
	* @param array	&$relations	relations
	* @param string	$prefix		prefix
	* @param int	$limit		limit
	*
	* @return void
	* @access private
	*/
	function _recurseCheckAllRelations(&$todo, &$relations, $prefix, $limit=NULL)
	{
		for (reset($todo); NULL !== ($k = key($todo)); next($todo)) {
			$el =& $todo[$k];
			if (is_array($el) && (is_null($limit) || ($limit > 0))) {

				$new_rel = Array('parent' => $prefix.$k, 'children' => Array());
				foreach (array_keys($el) as $el_key) {
					if ($limit > 1) {
						$new_rel['children'][] = $prefix.$k.'_'.$el_key;
					} else {
						$new_rel['children'][] = '['.$el_key.']';
					}
				}
				$relations[] = $new_rel;

				$this->_recurseCheckAllRelations($el, $relations, $prefix.$k.'_', (is_null($limit) ? NULL : $limit - 1));
			}
		}

	}//end _recurseCheckAllRelations()


	/**
	* Kick off analysing "check-all" relations
	*
	* The results of this function are cached in the _tmp member variable as
	* $this->_tmp['check_all_relations'].
	*
	* @param array	$todo	assets to check
	*
	* @return void
	* @access private
	*/
	function _analyseCheckAllRelations($todo)
	{
		if (!isset($this->_tmp['check_all_relations'])) {
			// if custom grouping is enabled and set up, we need to modify the
			// first level of groups to use the 'g' prefix
			if (($this->attr('group_by') == 'grouped') && (count($this->attr('asset_grouping')) > 0)) {
				$new_rel = Array('parent' => '_all', 'children' => Array());
				foreach (array_keys($todo) as $group_id) {
					$new_rel['children'][] = 'g_'.$group_id;
				}
			} else {
				$new_rel = Array('parent' => '_all', 'children' => Array());
				foreach (array_keys($todo) as $child) {
					$new_rel['children'][] = '['.$child.']';

				}
			}
			$relations = Array($new_rel);

			$this->_recurseCheckAllRelations($todo, $relations, 'g_', count($this->attr('asset_grouping')));
			$this->_tmp['check_all_relations'] =& $relations;
		}

		return $this->_tmp['check_all_relations'];

	}//end _analyseCheckAllRelations()


	/**
	* Build 'on click' code to ensure 'select all' functionality works
	*
	* @return void
	* @access private
	*/
	function _buildRelationsJS()
	{
		// order is important, and we actually need the 'ALL' one LAST so we need
		// to reverse this
		$rels = array_reverse($this->_tmp['check_all_relations']);
		$code = 'init_select_list_array(); select_list[\''.$this->getPrefix().'\'] = [';

		$select_lines = Array();
		foreach ($rels as $rel) {
			$parent_code = str_replace(' ', '_', $rel['parent']);
			for (reset($rel['children']); NULL !== ($k = key($rel['children'])); next($rel['children'])) {
				if (preg_match('|\[(.*)\]|', $rel['children'][$k], $matches)) {
					$rel['children'][$k] = $matches[1];
					$bracketed = TRUE;
				} else {
					$bracketed = FALSE;
				}

				if (is_numeric($rel['children'][$k])) {
					$rel['children'][$k] = str_replace(' ', '_', $rel['children'][$k]);
				} else {
					$rel['children'][$k] = make_valid_html_id(str_replace(' ', '_', $rel['children'][$k]));
				}

				if ($bracketed) {
					$rel['children'][$k] = '['.$rel['children'][$k].']';
				}
			}
			$select_lines[] = "[['".make_valid_html_id($parent_code)."'], ['".implode("', '", $rel['children'])."']]";
		}
		$code .= implode(', ', $select_lines);

		$code .= ']';

		return $code;

	}//end _buildRelationsJS()


	/**
	* Get the current form submit method
	*
	* @return string
	* @access protected
	*/
	function _getFormSubmitMethod()
	{
		return $this->attr('form_submit_method');

	}//end _getFormSubmitMethod()


//--		NO RESULTS BODYCOPY		   --//


	/**
	* Creates the no-results bodycopy
	*
	* @param boolean	$enable_on_create	if TRUE, will set the bodycopy to be enabled
	*										(TYPE_2 link) when it is created. If FALSE,
	*										creates it disabled (TYPE_3 link).
	*
	* @return boolean
	* @access public
	*/
	function createNoResultsBodycopy($enable_on_create=TRUE)
	{
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($bodycopy_links);

		// we already have a bodycopy link?!
		if ($bodycopy_link) {
			return FALSE;
		} else {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$link_type = ($enable_on_create ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3);

			$asset = new Bodycopy();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> 'no_results',
							'link_type'		=> $link_type,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$asset->setAttrValue('name', 'Page Contents (No Results)');
			$args = Array('content' => $this->_getDefaultBodycopyContent('no_results'));
			if (!$asset->create($copy_link, $args)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return TRUE;

	}//end createNoResultsBodycopy()


	/**
	* Returns the no-results bodycopy
	*
	* @param boolean	$only_if_enabled	if TRUE, will return NULL if the bodycopy is
	*										disabled. If FALSE, will only return NULL if
	*										never created.
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getNoResultsBodycopy($only_if_enabled=TRUE)
	{
		$asset = NULL;

		if ($only_if_enabled) {
			$link_types = SQ_LINK_TYPE_2;
		} else {
			$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;
		}

		$tmp_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($tmp_bodycopy_link);

		if ($bodycopy_link) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		}

		return $asset;

	}//end getNoResultsBodycopy()


	/**
	* Returns whether the no-results bodycopy is enabled
	*
	* @return boolean
	* @access public
	*/
	function isNoResultsBodycopyEnabled()
	{
		$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;

		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($bodycopy_links);

		if ($bodycopy_link) {
			return ($bodycopy_link['link_type'] == SQ_LINK_TYPE_2);
		} else {
			return FALSE;
		}


	}//end isNoResultsBodycopyEnabled()


	/**
	* Handles the enabling or disabling of the no-results bodycopy
	*
	* @param boolean	$enabled	set to TRUE to enable the format bodycopy,
	*								set to FALSE to disable it
	*
	* @return boolean
	* @access public
	*/
	function setUseNoResultsBodycopy($enabled)
	{
		$tmp_body_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($tmp_body_link);

		if (!$bodycopy_link) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if ($enabled) {
				if (!$this->createNoResultsBodycopy()) return FALSE;
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($bodycopy_link['link_type'] != $new_link_type) {
				$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], $new_link_type);
			}
		}

		return TRUE;

	}//end setUseNoResultsBodycopy()


	/**
	* Find and replace keyword modifiers in the listing page speical keyword replacements array
	*
	* @param array $replacements
	* @param string $assetid        The assetid to replace modifiers against. This is used when keywords are nested in the modifier arguments
	*
	* @return null
	* @access public
	*/
	function applyKeywordModifier(&$replacements, $assetid)
	{
		foreach ($replacements as $kw => $res) {
				$plain_keyword = parse_keyword($kw, $modifiers);
				// check to see if the keyword is actually replaced and
				// the replacement isn't just the keyword itself
				if(!empty($modifiers)) {
				    // If this is a keyword with modifiers, first see if context
				    // is one of them, and if so, augment the plain keyword
				    // so we get the version in the correct context
				    $contextid = extract_context_modifier($modifiers);
				    if ($contextid !== NULL) {
					$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
					$plain_keyword .= '^context:'.$all_contexts[$contextid]['name'];
				    }
				    $keywords_is_replaced = isset($replacements[$plain_keyword]) && $replacements[$plain_keyword] !== '%'.$plain_keyword.'%' && $replacements[$plain_keyword] !== $plain_keyword;
				    if ($keywords_is_replaced) {
					    $replacements[$kw] = $replacements[$plain_keyword];
					    // Apply modifiers
					    apply_keyword_modifiers($replacements[$kw], $modifiers,  Array('assetid' => $assetid, 'replacements' => $replacements));
				    }
				}
		}

	}//end applyKeywordModifier()


}//end class
?>
