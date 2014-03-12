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
* $Id: search_page.inc,v 1.269 2013/07/22 07:54:08 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Search_Page
*
* Page template for searching
*
* @see Page
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.269 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page extends Listing_Engine
{

	/**
	* An array of bodycopies and their names that are attached underneath this page asset
	* @var array
	*/
	var $bodycopies = Array(
						'initial'		=> 'Initial Search Page Layout',
						'results'		=> 'Results Page Layout',
						'no_results'	=> 'No Results Page Layout',
					  );

	/**
	* An array of default parameters
	*
	* @var array
	*/
	var $defaults = Array ('query_width' => 25);

	/**
	* An array of keywords replacements that can be used in this page
	*
	* @var array
	*/
	var $keywords = Array(
						'initial'		=> Array(
											'submit_button',
											'results_per_page',
											'structured_root_node',
										   ),
						'results'		=> Array(
											'result_list',
											'result_count',
											'current_result_page',
											'total_result_pages',
											'result_page_index',
											'previous_result_page_link',
											'next_result_page_link',
											'previous_result_page_href',
											'next_result_page_href',
											'initial_layout',
											'category_list_linked',
											'structured_root_node',
											'initial_result_count',
											'page_list_X'
										   ),
						'no_results'	=> Array(
											'initial_layout',
										   ),
					);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array (
	*	'asset'		   => [ref major asset to create link under],
	*	'link_type'    => SQ_LINK_?,
	*	'value'		   => [link value],
	*	'sort_order'   => [link sort order],
	*	'is_dependant' => [0|1],
	*	'is_exclusive' => [0|1],
	* )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// Grab the attributes lock
		if (!$lock_held = $GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
			return FALSE;
		}

		// Set the Sort Order to "Descending" by default, so the most relevant results are at the top (ref. Feature #2886)
		$this->setAttrValue('reverse_sort', TRUE);
		$this->saveAttributes();

		if ($lock_held == 1) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
		}

		return TRUE;

	}//end _createAdditional()


	/**
	* Creates bodycopies for this asset
	*
	* Override if different bodycopies are needed (eg. in search page).
	*
	* @return boolean
	* @access protected
	*/
	function _createBodycopies()
	{
		foreach ($this->bodycopies as $value => $name) {
			$copy_link = Array(
							'asset'			=> &$this,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
							'value'			=> $value,
						 );

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			$args = Array('content' => $this->_getDefaultBodycopyContent($value));
			if (!$bodycopy->create($copy_link, $args)) {
				return FALSE;
			}
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
			case 'initial':
				return '<p>'.translate('sch_page_search_for').'</p><p>%submit_button%</p>';
			case 'results':
				return '<p>'.translate('sch_page_x_results_found', '%result_count%').'</p>%result_list%';
			case 'no_results':
				return '<p>'.translate('sch_page_no_results_try_again').'</p>%initial_layout%';
		}
		return parent::_getDefaultBodycopyContent($bodycopy_code);

	}//end _getDefaultBodycopyContent()


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
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed string|boolean
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

		if ($bodycopy_link['minor_type_code'] == 'bodycopy' && in_array($bodycopy_link['value'], $this->bodycopies) && !$GLOBALS['SQ_PURGING_TRASH']) {
			return translate('sch_page_cannot_delete_bodycopies');
		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 'M', 'exclusive' => TRUE);
		$page_links[SQ_LINK_TYPE_2]['folder']   = Array('card' => 4, 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['folder']   = Array('card' => 1, 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['asset']    = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


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
				return translate('sch_page_root_node_link_desc');
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'prepareSearchQuery');

		if (isset($_REQUEST[$this->getPrefix().'_submit_button']) && empty($_REQUEST['mode'])) {
			$_REQUEST['mode'] = 'results';
		}

		$mode = array_get_index($_REQUEST, 'mode', '');

		if (empty($mode)) {
			// we dont know what button was pressed, so see if we have any query vars
			// that show evidence that a search has been performed without submitting
			// the button value
			$queries = $this->_getSearchedQueries();
			foreach ($queries as $field_name => $value) {
				if (!empty($value)) {
					$mode = 'results';
					break;
				} else {
					unset($queries[$field_name]);
				}
			}

			$this->_tmp['searched_queries'] = $queries;
		}

		if (empty($_SESSION['SQ_LAST_SEARCH'][$this->id])) {
			$_SESSION['SQ_LAST_SEARCH'][$this->id] = Array();
		} else {
			switch ($mode) {
				case 'sis':
					// do nothing
				break;

				case 'results':
					// preserve the previous query
					$this->_tmp['potential_searched_queries'] = $_SESSION['SQ_LAST_SEARCH'][$this->id];

					// we intentionally fall-through to default
				default:
					$_SESSION['SQ_LAST_SEARCH'][$this->id] = Array();
				break;
			}
		}

		if (empty($mode)) $mode = 'initial';

		// get the corresponding bodycopy attached to us
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		$listing_result_page = (int) array_get_index($_REQUEST, 'result_'.$this->id.'_result_page', 0);
		$search_result_page = (int) array_get_index($_REQUEST, 'current_result_page', 1);
		if ($listing_result_page === 0) {
			$current_result_page = $search_result_page;
		} else {
			$current_result_page = $listing_result_page;
		}//end if
		$results_per_page    = (int) array_get_index($_REQUEST, 'results_per_page', $this->_getNumPerPage());
		$submitted_category  = htmlentities(array_get_index($_REQUEST, 'search_category', array_get_index($_REQUEST, 'submitted_search_category')));

		if (empty($_REQUEST['search_category']) && !empty($submitted_category)) {
			$_REQUEST['search_category'] = $submitted_category;
		}

		// Set this variable to pass to the processSearch() function
		$search_vars = $this->processSearchVars();

		// If Stored Query is to be used, then jump to the results page directly
		if ($this->attr('stored_query_show_results') && $mode == 'initial' && $this->_tmp['jump_to_result']) {
			$mode = 'results';
		}//end if

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'prepareSearchQuery');

		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'printBody');

		switch ($mode) {
			case 'results':
			case 'sis'    :
				$this->printResultsBody($search_vars);
			break;

			case 'initial':
			default:
				echo $this->getBodycopyContents('initial', $this->getInitialBodyReplacements());
			break;
		}

		// only print if submit buttons are printed
		if (!empty($this->_tmp['prints_submit'])) {
			?>
			<div>
			<input type="hidden" name="current_result_page" value="<?php echo $current_result_page; ?>" />
			<input type="hidden" name="results_per_page" value="<?php echo $results_per_page; ?>" />
			<?php
			$this->registerFormField('result_page');
			$this->registerFormField('current_result_page');
			$this->registerFormField('results_per_page');

			?>
			<input type="hidden" name="submitted_search_category" value="<?php echo $submitted_category; ?>" />
			<input type="hidden" name="mode" value="" />
			</div>
			<?php
			$this->registerFormField('submitted_search_category');
			$this->registerFormField('mode');
		}

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printBody');
	}//end printContents()


	/**
	* Prints the results bodycopy
	*
	* @param array	$search	The passed search variable used for stored searches
	*
	* @return void
	* @access private
	*/
	function printResultsBody($search=Array())
	{
		$results = $this->processSearch($search);

		// exclude root nodes themselves if it is required
		if ($this->attr('exclude_self')) {
			$root_nodes = $this->getRootNodes();
			foreach ($root_nodes as $exclude_assetid) {
				if (isset($results[$exclude_assetid])) {
					unset($results[$exclude_assetid]);
				}
			}
		}

		if($this->attr('exclude_current_asset')) {
			foreach ($results as $assetid => $data) {
				$url = strip_url($GLOBALS['SQ_SYSTEM']->am->getAssetURL($assetid));
				$requester = strip_url(current_url());
				if ($requester === $url) {
					unset($results[$assetid]);
				}
			}
		}

		$this->_tmp['initial_results'] = $results;
		$results =& $this->convertProxyAssetTypes($results);
		$this->_tmp['search_results'] =& $results;
		$bc_name = 'results';
		if (empty($results)) $bc_name = 'no_results';

		$bodycopy =& $this->getBodycopy($bc_name);
		if (is_null($bodycopy)) {
			return Array();
		}
		$keywords = $bodycopy->getKeywords();

		echo $this->getBodycopyContents($bc_name, $this->getResultsBodyReplacements($results, $bc_name));

		// there is no asset listing keyword in the contents, so we dont
		// need to do all the extra processing for the listing
		if (!in_array('select_all_js_code', $keywords)) {
			echo $this->getSelectAllJSCodeKeywordReplacement();
		}

	}//end printResultsBody()


	/**
	* Remove assets with a status that we do not want to show
	*
	* Search page doesnt need to filter anything because Search Manager does
	* everything for it, including checking read access.
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
		return;

	}//end filterAssetStatuses()


	/**
	* Sort the list of assets
	*
	* @param array	$asset_list	an array of assets to be sorted
	* @param array	$sort_info	sorting information as provided by getSortInfo()
	*
	* @return array
	* @access public
	* @see Listing_Engine::sortAssetList()
	* @see Listing_Engine::getSortInfo()
	*/
	function sortAssetList($asset_list, $sort_info)
	{
		if (count($asset_list) == 1 || empty($sort_info)) {
			return $asset_list;
		}

		if ((!isset($sort_info['type']) || ($sort_info['type'] == '')) && (!isset($sort_info['params']['field']) || $sort_info['params']['field'] == '')) {
			if (!empty($asset_list)) arsort($asset_list);
		}

		// Search Page-specific sorting
		if (isset($sort_info['params']['field'])) {
			$reverse_sort = $this->isDescending();

			// sort assets by score (relevance)
			if ($sort_info['params']['field'] == 'score') {
				if ($reverse_sort) {
					arsort($asset_list);
				} else {
					asort($asset_list);
				}

				return $asset_list;
			}
		}

		return parent::sortAssetList($asset_list, $sort_info);

	}//end sortAssetList()


	/**
	* Retrieves a bodycopy by name
	*
	* Returns the bodycopy, or NULL if the name does not match any bodycopies
	*
	* @param string	$name	the name of the bodycopy to retrieve
	*
	* @return mixed object|NULL
	* @access private
	* @see Search_Page::bodycopies
	*/
	function &getBodycopy($name)
	{
		$bodycopy = NULL;

		if (array_key_exists($name, $this->bodycopies)) {
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name, 'major', '1');
			 if (!empty($link)) {
				$bodycopy = $am->getAsset($link['minorid'], $link['minor_type_code']);
			}
		}

		return $bodycopy;

	}//end getBodycopy()


	/**
	* Retrieves a bodycopy's contents by name
	*
	* @param string	$name			the name of the bodycopy to retrieve
	* @param array	$replacements	replacements
	*
	* @return mixed string|NULL
	* @access private
	* @see Search_Page::bodycopies
	* @see Search_Page::getBodycopy
	*/
	function &getBodycopyContents($name, $replacements=Array())
	{
		$html = '';
		$bodycopy =& $this->getBodycopy($name);
		if (is_null($bodycopy)) {
			return $html;
		}

		$bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getBodycopyContents()


	/**
	* Returns true if the datasources for the supplied field imply more than one input style
	*
	* Different input styles are implied if there are datasources of more than one type, or if
	* the attributes for several asset-attribute datasources are of different types.  They are
	* also implied if a selection attribute or selection metadata field is found, since the input
	* style for these is specific to the particular attribute or field.
	*
	* @param array	$field	Details of a field from the fields attr
	*
	* @return boolean
	* @access public
	*/
	function _hasMixedFieldTypes($field)
	{
		$mixed_ds_types = FALSE;
		if (count($field['data_sources']) > 1) {
			// several data sources, see what we can do
			$first_ds_type = NULL;
			$first_field_type = NULL;
			foreach ($field['data_sources'] as $ds) {
				if (is_null($first_ds_type)) {
					$first_ds_type = $ds['type'];
				}
				if ($ds['type'] != $first_ds_type) {
					$mixed_ds_types = TRUE;
					break;
				}
				$this_field_type = NULL;
				switch ($ds['type']) {
					case 'metadata':
						$assetid = $ds['params']['assetid'];
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if (!is_null($asset)) {
							$this_field_type = get_class_lower($asset);
						}
					break;
					case 'asset_attrib':
						$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($ds['params']['attrid']);
						$this_field_type = $attribute->type();
					break;
					case 'standard' :
						$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
						if (isset($sm->standard_date_fields[$ds['params']['field']])) {
							$this_field_type = 'date';
						} else {
							$this_field_type = 'text';
						}
					break;
				}
				if (is_null($first_field_type)) {
					$first_field_type = $this_field_type;
				}
				if ($this_field_type != $first_field_type) {
					$mixed_ds_types = TRUE;
					break;
				}
				if (in_array(strtolower($this_field_type), Array('selection', 'metadata_field_select', 'metadata_field_hierarchy'))) {
					$mixed_ds_types = TRUE;
					break;
				}
			}//end foreach
		}//end if
		return $mixed_ds_types;

	}//end _hasMixedFieldTypes()


	/**
	* Get a datetime field for painting and processing
	*
	* @param string	$field_name	the name of the field we are creating
	*
	* @return object
	* @access private
	*/
	function &_getDateField($field_name)
	{
		$parameters = Array(
						'min'			=> '0000-01-01 00:00:00',
						'max'			=> '9999-12-31 23:59:59',
						'allow_circa'	=> '0',
						'print_format'	=> '',
						'show'			=> Array('y', 'm', 'd'),
						'null'			=> Array('y', 'm', 'd'),
						'style'			=> Array(
											'y'	=> 't',
											'm'	=> 's',
											'd'	=> 's',
										   ),
					  );

		$value = '---------- --:--:--';
		$field = new DateTime_Field($field_name, $value, $parameters, TRUE);
		return $field;

	}//end _getDateField()


	/**
	* Print a description for a given date range
	*
	* @param array	$current_value	the values that should be printed instead of the date selection boxes
	*
	* @return void
	* @access private
	*/
	function _printDateRangeDescription($current_value)
	{
		if (!isset($current_value['from'])) {
			$current_value['from'] = '---------- --:--:--';
		}

		if (!isset($current_value['to'])) {
			$current_value['to']   = '---------- --:--:--';
		}

		$from_string = '';
		if ($current_value['from'] != '---------- --:--:--') {
			$from = iso8601_ts($current_value['from']);
			$from_string = date('j M Y', $from);
		}

		$to_string = '';
		if ($current_value['to'] != '---------- --:--:--') {
			$to = iso8601_ts($current_value['to']);
			$to_string = date('j M Y', $to);
		}

		if (empty($from_string) && empty($to_string)) {
			return;
		} else if (empty($from_string)) {
			echo translate('sch_page_date_range_prior_to', $to_string);
		} else if (empty($to_string)) {
			echo translate('sch_page_date_range_onwards', $from_string);
		} else if ($from_string == $to_string) {
			echo $from_string;
		} else {
			echo $from_string.' - '.$to_string;
		}

	}//end _printDateRangeDescription()


	/**
	* Print a date range selection
	*
	* @param string		$field_name		the name of the field we are painting
	* @param array		$current_value	the values that should be printed instead of the date selection boxes
	* @param boolean	$read_only		should we print read only if we have a value
	*
	* @return void
	* @access private
	*/
	function _printDateRange($field_name, $current_value=Array(), $read_only=FALSE)
	{
		if (!isset($current_value['from'])) {
			$current_value['from'] = '---------- --:--:--';
		}

		if (!isset($current_value['to'])) {
			$current_value['to']   = '---------- --:--:--';
		}

		if ($this->attr('use_picker')) {
			require_once SQ_LIB_PATH.'/js_calendar/js_calendar.inc';
			if (!JS_Calendar::isInit()) {
				?><script src="<?php echo sq_web_path('lib'); ?>/html_form/html_form.js"
				type="text/javascript"></script><?php
			}
		}

		?>
		<table border="0" cellspacing="0" cellpadding="0">
			<tr>
				<td align="right"><?php echo ucfirst(translate('from')) ?>:</td>
				<td>
					<?php
					if ($read_only && $current_value['from'] != '---------- --:--:--') {
						$from = iso8601_ts($current_value['from']);
						echo date('j M Y', $from);
					} else {
						$field = $this->_getDateField($field_name.'_from');
						if (!$field->processField() || $field->value == '---------- --:--:--') {
							$field->setValue($current_value['from']);
						}
						$field->printField();
					}
					?>
				</td>
				<?php
				if ($this->attr('use_picker') && (!$read_only || $current_value['from'] == '---------- --:--:--')) {
					?>
					<td>
						&nbsp;
						<?php
						// print the JS calendar popup date selecta
						$calendar = new JS_Calendar();
						$calendar->changeSetting('onDayClick', 'datetime_set_date');
						$calendar->paint($field_name.'_from', '', TRUE);
						?>
					</td>
					<?php
				}
				?>
			</tr>
			<tr>
				<td align="right"><?php echo ucfirst(translate('to')) ?>:</td>
				<td>
					<?php
					if ($read_only && $current_value['to'] != '---------- --:--:--') {
						$to = iso8601_ts($current_value['to']);
						echo date('j M Y', $to);
					} else {
						$field = $this->_getDateField($field_name.'_to');
						if (!$field->processField() || $field->value == '---------- --:--:--') {
							$field->setValue($current_value['to']);
						}
						$field->printField();
					}
					?>
				</td>
				<?php
				if ($this->attr('use_picker') && (!$read_only || $current_value['to'] == '---------- --:--:--')) {
					?>
					<td>
						&nbsp;
						<?php
						// print the JS calendar popup date selecta
						$calendar = new JS_Calendar();
						$calendar->changeSetting('onDayClick', 'datetime_set_date');
						$calendar->paint($field_name.'_to', '', TRUE);
						?>
					</td>
					<?php
				}
				?>
			</tr>
		</table>
		<?php

	}//end _printDateRange()


	/**
	* Print a date range selection
	*
	* @param string	$field_name	the name of the field we are processing
	*
	* @return array
	* @access private
	*/
	function _processDateRange($field_name)
	{
		$result = Array();

		$field = $this->_getDateField($field_name.'_from');
		if ($field->processField()) {
			$result['from'] = $field->value;
		}

		$field = $this->_getDateField($field_name.'_to');
		if ($field->processField()) {
			$result['to'] = $field->value;
		}

		return $result;

	}//end _processDateRange()


	/**
	* Print a description for a given numeric range
	*
	* @param array	$current_value	the values that should be printed
	* @param array	$sub_field		if empty, we are printing %field_name%
	*								if not empty, we are printing %field_name_subfield%
	* @param array	$params			the data source's params list, needed so we know what we're allowed to print
	*
	* @return void
	* @access private
	*/
	function _printNumericRangeDescription($current_value, $sub_field, $params)
	{
		switch (array_get_index($params, 'numeric_search', 'exact')) {
			case 'exact':
				if ($sub_field == '') print $current_value['lower'];
			break;

			case 'range':
				if (!isset($current_value['upper'])) {
					$current_value['upper'] = NULL;
				}

				if (!isset($current_value['lower'])) {
					$current_value['lower'] = NULL;
				}

				if ($sub_field == 'lower') {
					print array_get_index($current_value, 'lower', '');
				} else if ($sub_field == 'upper') {
					print array_get_index($current_value, 'upper', '');
				} else {
					if (is_null($current_value['lower'])) {
						echo '&lt;= '.$current_value['upper'];
					} else if (is_null($current_value['upper'])) {
						echo '&gt;= '.$current_value['lower'];
					} else {
						echo $current_value['lower'].' to '.$current_value['upper'];
					}
				}
			break;

			case 'custom':
				$custom_ranges = Array();
				$range_name = '';
				foreach ($params['custom_num_search'] as $key => $num_search_data) {
					if (!empty($num_search_data['name']) && ($current_value['lower'] == $num_search_data['min']) && ($current_value['upper'] == $num_search_data['max'])) {
						$range_name = $num_search_data['name'];
						break;
					}
				}

				if ($sub_field == 'lower') {
					print array_get_index($current_value, 'lower', '');
				} else if ($sub_field == 'upper') {
					print array_get_index($current_value, 'upper', '');
				} else if ($sub_field == '') {
					print $range_name;
				}
			break;

		}//end switch

	}//end _printNumericRangeDescription()


	/**
	* Print a numeric range selection
	*
	* Note: this brings in a "sub-field" specification - we need this to be
	* able to print the upper and lower values of a 'range' search separately.
	*
	* @param string		$field_name		the name of the field we are painting
	* @param array		$current_value	the values that should be printed instead of the date selection boxes
	* @param array		$params			the data source's params list, needed so we know what we're allowed to print
	* @param array		$sub_field		if empty, we are printing %field_name%
	*									if not empty, we are printing %field_name_subfield%
	* @param boolean	$read_only		should we print read only if we have a value
	*
	* @return void
	* @access private
	*/
	function _printNumericRange($field_name, $current_value=NULL, $params, $sub_field='', $read_only=FALSE)
	{
		switch (array_get_index($params, 'numeric_search', 'exact')) {
			case 'exact':
				if ($sub_field == '') {
					text_box('queries_'.$field_name, array_get_index($current_value, 'lower', ''), 5);
				}
			break;

			case 'range':
				$current_value = NULL;

				if (!isset($current_value['upper'])) {
					$current_value['upper'] = NULL;
				}

				if (!isset($current_value['lower'])) {
					$current_value['lower'] = NULL;
				}

				if ($sub_field == 'lower') {
					text_box('queries_'.$field_name.'_lower', $current_value['lower'], 5);
				} else if ($sub_field == 'upper') {
					text_box('queries_'.$field_name.'_upper', $current_value['upper'], 5);
				} else {
					text_box('queries_'.$field_name.'_lower', $current_value['lower'], 5);
					echo ' to ';
					text_box('queries_'.$field_name.'_upper', $current_value['upper'], 5);
				}
			break;

			case 'custom':
				$custom_ranges = Array();
				foreach ($params['custom_num_search'] as $key => $num_search_data) {
					if (!empty($num_search_data['name'])) {
						$custom_ranges[$key] = $num_search_data['name'];
					}
				}
				if ($sub_field == '') {
					combo_box('queries_'.$field_name, $custom_ranges, FALSE, $current_value);
				}
			break;

		}//end switch

	}//end _printNumericRange()


	/**
	* Get Category Chooser Replacement
	*
	* @param string		$cat_set_name		Category Name
	* @param array		$cat_set_details	Category details
	* @param string		$default			Default value of the category
	* @param boolean	$show_empty			Whether to show the 'unselected text'
	*
	* @return void
	* @access public
	*/
	function getCategoryChooserReplacement($cat_set_name, $cat_set_details, $default='', $show_empty=TRUE)
	{
		$prefix = $this->getPrefix();
		$options = Array();

		if ($show_empty) {
			$options[''] = $cat_set_details['unselected_text'];
		}

		foreach ($cat_set_details['options'] as $code_name => $cat_option) {
			$options[$code_name] = $cat_option['full_name'];
		}
		ob_start();
			$this->registerFormField('category_'.$cat_set_name);
			combo_box('category_'.$cat_set_name, $options, FALSE, $default);
			$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getCategoryChooserReplacement()


	/**
	* Returns the replacement for the specified field
	*
	* @param string	$field_name	the name of the field to get the replacement for
	* @param string	$sub_field	sub field
	*
	* @return string
	* @access public
	*/
	function getFieldReplacement($field_name, $sub_field='')
	{
		$query_var = $field_name.'_query';
		$fields = $this->attr('fields');
		if (!isset($fields[$field_name])) return;
		$this->registerFormField('queries_'.$query_var.'%');
		$query = '';

		if (isset($_SESSION['SQ_LAST_SEARCH'][$this->id])) {
			$query = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, '');
		}
		$read_only = FALSE;

		// Check if a stored stored session variable is set and set query to match
		// But don't touch it if there was some SESSION var already set
		$pmap = $this->getAttribute('stored_query_session');
		$pmap_params = $pmap->getParameters();
		if (empty($query) && is_array($pmap_params) && !empty($pmap_params)) {
			// Check for all fields
			if (in_array('stored_query', $pmap_params)) {
				$key_field = array_search('stored_query', $pmap_params);
				$query = $pmap->getParameterValue($pmap_params[$key_field]);
			}

			// Check for current field
			if (in_array($field_name, $pmap_params)) {
				$key_field = array_search($field_name, $pmap_params);
				$query = $pmap->getParameterValue($pmap_params[$key_field]);
			}
			replace_global_keywords($query);
		}

		ob_start();

			if (empty($fields[$field_name]['data_sources']) || $this->_hasMixedFieldTypes($fields[$field_name])) {
				text_box('queries_'.$query_var, $query, $this->defaults['query_width']);
			} else {
				// only one data source, or all matching; we'll base our format on the first one
				$data_source = $fields[$field_name]['data_sources'][0];

				switch ($data_source['type']) {

					case 'metadata' :
						$assetid = $data_source['params']['assetid'];
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

						if (!is_null($asset)) {
							$params = $asset->vars;
							switch (get_class_lower($asset)) {
								case 'metadata_field_date' :
									$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
									$this->_printDateRange('queries_'.$query_var, $current_value);
								break;
								case 'metadata_field_text' :
								case 'metadata_field_thesaurus':
								case 'metadata_field_multiple_text':
								case 'metadata_field_wysiwyg':
									text_box('queries_'.$query_var, $query, $this->defaults['query_width']);
								break;
								case 'metadata_field_hierarchy' :
									$hierarchy = $asset->getHierarchyAttribute();

									if (!empty($query)) $hierarchy->value = $query;
									if ($asset->attr('hier_sort')) {
										$sort = $asset->attr('hier_sort');
										$fmt = $asset->attr('hier_structure');
										$edit_fns = $asset->getEditFns();
										$options = $hierarchy->_params['options'];
										$options = $edit_fns->_sortHierarchyOptions($options, $sort, $fmt);
										$hierarchy->_params['options'] = $options;
									}
									$hierarchy->paint('queries_'.$query_var);
								break;
								case 'metadata_field_select' :
									$selection = $asset->getSelectionAttribute();
									if (!empty($query)) $selection->setValue($query);
									$selection->paint('queries_'.$query_var);
								break;
							}//end switch
						} else {
							// not printing a field, set read-only
							$read_only = TRUE;
						}//end if metadata asset exists
					break;

					case 'asset_attrib' :
						$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);

						if ($attribute->type() == 'datetime') {

							$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
							$this->_printDateRange('queries_'.$query_var, $current_value);

						} else if (($attribute->type() == 'int' || $attribute->type() == 'float')) {
							$this->_printNumericRange($query_var, $query, $data_source['params'], $sub_field);

						} else {

							// if this is a boolean or selection field, we want to make sure blank options are allowed
							if ($attribute->type() == 'boolean' || $attribute->type() == 'selection') {
								$attribute->_params['allow_empty'] = TRUE;
							}

							// set the current value of the search field
							@$attribute->setValue($query);

							// give the attribute our custom search styles if we have them
							$search_styles = $this->attr('search_styles');
							if (isset($search_styles[$attribute->id])) {
								$attribute->_edit_params = array_merge($attribute->_edit_params, $search_styles[$attribute->id]);
							}

							$attribute->paint('queries_'.$query_var, FALSE);
						}
					break;

					case 'standard' :
						$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
						if (isset($sm->standard_date_fields[$data_source['params']['field']])) {
							$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
							$this->_printDateRange('queries_'.$query_var, $current_value);
						} else if (isset($sm->standard_text_fields[$data_source['params']['field']])) {
							text_box('queries_'.$query_var, $query, $this->defaults['query_width']);
						}
					break;

					default :
						text_box('queries_'.$query_var, $query, $this->defaults['query_width']);
					break;

				}//end switch field type
			}//end else

			if (!$read_only) {
				$this->registerFormField('queries_'.$query_var);
			}

			$replacement = ob_get_contents();

		ob_end_clean();

		return $replacement;

	}//end getFieldReplacement()


	/**
	* Returns the replacement for the specified field in a search in search form
	*
	* @param string	$field_name	the name of the field to get the replacement for
	*
	* @return string
	* @access public
	*/
	function getSisFieldReplacement($field_name)
	{
		$query_var = $field_name.'_query_sis';
		$query = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, '');
		$fields = $this->attr('fields');
		$read_only = FALSE;

		ob_start();

			if (!$this->_hasMixedFieldTypes($fields[$field_name])) {
				$data_source = $fields[$field_name]['data_sources'][0];

				switch ($data_source['type']) {

					case 'metadata' :
						$assetid = $data_source['params']['assetid'];
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if (!is_null($asset)) {
							$params = $asset->vars;
							$this->registerFormField('queries_'.$query_var);
							switch (get_class_lower($asset)) {
								case 'metadata_field_date' :
									$read_only = TRUE;
									$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
									$this->_printDateRange('queries_'.$query_var, $current_value, TRUE);
								break;
								case 'metadata_field_text' :
								case 'metadata_field_thesaurus':
								case 'metadata_field_multiple_text':
									text_box('queries_'.$query_var, '', $this->defaults['query_width']);
								break;
								case 'metadata_field_hierarchy' :
									$hierarchy = $asset->getHierarchyAttribute();
									if (!empty($query)) $hierarchy->value = $query;
									if ($asset->attr('hier_sort')) {
										$sort = $asset->attr('hier_sort');
										$fmt = $asset->attr('hier_structure');
										$edit_fns = $asset->getEditFns();
										$options = $hierarchy->_params['options'];
										$options = $edit_fns->_sortHierarchyOptions($options, $sort, $fmt);
										$hierarchy->_params['options'] = $options;
									}
									$hierarchy->paint('queries_'.$query_var);
								break;
								case 'metadata_field_select' :
									$selection = $asset->getSelectionAttribute();
									$value = Array();

									if (empty($selection->_params['multiple'])) {
										if (isset($selection->_params['options'][$query])) {
											$read_only = TRUE;
										}
										// set the value outside the if condition so that if
										// empty text has been selected, it will get saved as the option,
										// but the selection attribute will NOT be read only ie. they
										// will be able to select another option in the sis form
										$selection->value = $query;
									} else {
										if (!empty($query)) {
											echo '<div class="search_current_options">';
											foreach ($query as $option) {
												if (!isset($selection->_params['options'][$option])) {
													continue;
												}
												echo $selection->_params['options'][$option].'<br />';
												unset($selection->_params['options'][$option]);
											}
											echo '</div>';
										}
									}

									$selection->paint('queries_'.$query_var, $read_only);
								break;
							}//end switch

						} else {
							// not printing a field, set read-only
							$read_only = TRUE;
						}//end if metadata asset exists
					break;

					case 'asset_attrib' :
						$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);

						if ($attribute->type() == 'datetime') {

							$read_only = TRUE;
							$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
							$this->_printDateRange('queries_'.$query_var, $current_value, $read_only);

						} else {

							// if this is a boolean or selection field, we want to make sure blank options are allowed
							if ($attribute->type() == 'boolean' || $attribute->type() == 'selection') {
								$attribute->_params['allow_empty'] = TRUE;
							}

							// give the attribute our custom search styles if we have them
							$search_styles = $this->attr('search_styles');
							if (isset($search_styles[$attribute->id])) {
								$attribute->_edit_params = $search_styles[$attribute->id];
							}

							switch ($attribute->type()) {
								case 'selection' :
									if (empty($attribute->_params['multiple'])) {
										if (isset($attribute->_params['options'][$query])) {
											$read_only = TRUE;
										}
									} else {
										if (!empty($query)) {
											echo '<div class="search_current_options">';
											foreach ($query as $option) {
												if (!isset($attribute->_params['options'][$option])) {
													continue;
												}
												echo $attribute->_params['options'][$option].'<br />';
												unset($attribute->_params['options'][$option]);
											}
											echo '</div>';
										}
									}
								break;

								case 'boolean' :
									if ($query != '') $read_only = TRUE;
								break;

								case 'text' :
									$attribute->value = '';
								break;
							}//end switch

							$attribute->paint('queries_'.$query_var, $read_only);
						}//end else
					break;

					case 'standard' :
						$read_only = TRUE;
						$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
						if (isset($sm->standard_date_fields[$data_source['params']['field']])) {
							$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
							$this->_printDateRange('queries_'.$query_var, $current_value, $read_only);
						} else if (isset($sm->standard_text_fields[$data_source['params']['field']])) {
							text_box('queries_'.$query_var, '', $this->defaults['query_width']);
						}
					break;

					default :
						// not a metadata field, just a normal text field
						text_box('queries_'.$query_var, '', $this->defaults['query_width']);
					break;

				}//end switch field type
			} else {
				// when there are multiple (or 0) data sources supplied the format is always text (for now)
				text_box('queries_'.$query_var, $query, $this->defaults['query_width']);
			}

			if (!$read_only) {
				$this->registerFormField('queries_'.$query_var);
			}

			$replacement = ob_get_contents();

		ob_end_clean();

		return $replacement;

	}//end getSisFieldReplacement()


	/**
	* Returns the current search terms for the passed field
	*
	* @param string		$field_name	the name of the field to get the replacement for
	* @param string		$sub_field	sub field
	* @param boolean	$escape		if true, then escape the value to return using htmlspecialchars()
	*
	* @return string
	* @access public
	*/
	function getSearchedFieldReplacement($field_name, $sub_field='', $escape=TRUE)
	{
		$query_var = $field_name.'_query_sis';
		$query = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, '');
		$fields = $this->attr('fields');

		ob_start();

			if (!$this->_hasMixedFieldTypes($fields[$field_name])) {
				switch ($fields[$field_name]['data_sources'][0]['type']) {

					case 'metadata' :
						$assetid = $fields[$field_name]['data_sources'][0]['params']['assetid'];
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if (!is_null($asset)) {
							$params = $asset->vars;
							switch (get_class_lower($asset)) {
								case 'metadata_field_date' :
									$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
									$this->_printDateRangeDescription($current_value);
								break;
								case 'metadata_field_text' :
								case 'metadata_field_thesaurus':
								case 'metadata_field_multiple_text':
									echo ($escape ? htmlspecialchars($query) : $query);
								break;
								case 'metadata_field_hierarchy' :
									$hierarchy = $asset->getHierarchyAttribute();
									if (!empty($query)) $hierarchy->value = $query;
									if ($asset->attr('hier_sort')) {
										$sort = $asset->attr('hier_sort');
										$fmt = $asset->attr('hier_structure');
										$edit_fns = $asset->getEditFns();
										$options = $hierarchy->_params['options'];
										$options = $edit_fns->_sortHierarchyOptions($options, $sort, $fmt);
										$hierarchy->_params['options'] = $options;
									}
									$hierarchy->paint('queries_'.$query_var);
								break;
								case 'metadata_field_select' :
									// NOTE: if the selection field allows multiple selections, then the selection cannot be paint()ed unescaped
									$selection = $asset->getSelectionAttribute();
									if (!empty($query)) $selection->setValue($query);
									$selection->paint('queries_'.$query_var, TRUE);
								break;
							}//end switch

						}//end if metadata asset exists
					break;

					case 'asset_attrib' :
						$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($fields[$field_name]['data_sources'][0]['params']['attrid']);

						if ($attribute->type() == 'datetime') {

							$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
							$this->_printDateRangeDescription($current_value);

						} else if (($attribute->type() == 'int' || $attribute->type() == 'float')) {
							$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $field_name, Array());
							$this->_printNumericRangeDescription($current_value, $sub_field, $fields[$field_name]['data_sources'][0]['params']);

						} else {
							// if this is a boolean or selection field, we want to make sure blank options are allowed
							if ($attribute->type() == 'boolean' || $attribute->type() == 'selection') {
								$attribute->_params['allow_empty'] = TRUE;
							}

							// give the attribute our custom search styles if we have them
							$search_styles = $this->attr('search_styles');
							if (isset($search_styles[$attribute->id])) {
								$attribute->_edit_params = $search_styles[$attribute->id];
							}

							$attribute->value = $query;

							$attribute->paint('queries_'.$query_var, TRUE);
						}
					break;

					default :
						if (is_array($query) && isset($query['from']) && isset($query['to'])) {
							// this is a date field
							$this->_printDateRangeDescription($query);
						} else {
							// just a standard text field
							echo ($escape ? htmlspecialchars($query) : $query);
						}
					break;

				}//end switch field type
			} else {
				// many (or zero) data sources for this field, so we'll assume it was just text
				echo ($escape ? htmlspecialchars($query) : $query);
			}

			$replacement = ob_get_contents();

		ob_end_clean();

		return $replacement;

	}//end getSearchedFieldReplacement()


	/**
	* Returns replacements that that are common to the bodycopies
	*
	* @param string	$keyword	the keyword of the wanted replacement
	*
	* @return string
	* @access public
	*/
	function getGeneralReplacement($keyword)
	{
		$prefix = $this->getPrefix();
		$replacement = '';
		switch ($keyword) {

			case 'search_button' :
			case 'submit_button' :
				$field_name = $prefix.'_submit_button';
				$this->registerFormField($field_name);
				$this->_tmp['prints_submit'] = TRUE;

				// if we are on the initial search page, don't use javascript
				// for the submit button so enter will submit the form
				$mode = array_get_index($_REQUEST, 'mode', 'initial');
				// Extra hidden field to make IE submit the form with a single field (See bug #5279)
				$replacement = '<input type="text" name="'.$prefix.'_extra_field" id="'.$prefix.'_extra_field" disabled="disabled" style="display:none" />';
				if ($mode == 'initial') {
					$replacement  .= '<input type="submit" name="'.$field_name.'"
						value="'.$this->attr('submit_button_text').'" />';
				} else {
					$replacement  .= '<input type="submit" name="'.$field_name.'" onclick="listing_form = document.getElementById(\''.$this->getPrefix().'\');
						listing_form.current_result_page.value=\'1\';
						if (typeof listing_form.mode != \'undefined\') listing_form.mode.value = \'results\';
						return true;
					" value="'.$this->attr('submit_button_text').'" />';
				}
			break;

			case 'search_in_search_button' :
				$field_name = 'sis_search_button';
				$this->registerFormField($field_name);
				$this->_tmp['prints_submit'] = TRUE;

				$replacement  = '<input type="submit" name="'.$field_name.'" onclick="listing_form = document.getElementById(\''.$this->getPrefix().'\');
					listing_form.current_result_page.value=\'1\';
					listing_form.mode.value = \'sis\';
					return true;
				" value="'.$this->attr('sis_button_text').'" />';
			break;

			case 'search_form':
				$bodycopy_link = $this->getFormatBodycopyLink('search_form');

				if (!empty($bodycopy_link) && $bodycopy_link['link_type'] == SQ_LINK_TYPE_2) {
					$replacements = $this->getFormFormatReplacements('search_form');
					$replacement = $this->getFormFormatContents('search_form', $replacements);
				} else {
					$replacement = '';
				}
			break;

			case 'search_in_search_form':
				$bodycopy_link = $this->getFormatBodycopyLink('sis_form');

				if (!empty($bodycopy_link) && $bodycopy_link['link_type'] == SQ_LINK_TYPE_2) {
					$replacements = $this->getFormFormatReplacements('sis_form');
					$replacement = $this->getFormFormatContents('sis_form', $replacements);
				} else {
					$replacement = '';
				}
			break;

			case 'results_per_page' :
				$results_per_page = array_get_index($_REQUEST, 'results_per_page', $this->_getNumPerPage());
				$field_name = 'results_per_page_text_box';
				$this->registerFormField($field_name);
				$replacement = '<input type="text" name="'.$field_name.'" onchange="listing_form = document.getElementById(\''.$this->getPrefix().'\');
					listing_form.results_per_page.value = this.value;
				" value="'.$results_per_page.'" size="5" />';
			break;

			default :
				$replacement = $this->getKeywordReplacement($keyword);
			break;

		}//end switch

		return $replacement;

	}//end getGeneralReplacement()


	/**
	* Prints the initial bodycopy
	*
	* @return void
	* @access private
	*/
	function getInitialBodyReplacements()
	{
		$bodycopy =& $this->getBodycopy('initial');
		if (is_null($bodycopy)) {
			return Array();
		}

		$keywords = $bodycopy->getKeywords();
		$replaces = Array();

		$fields = $this->attr('fields');
		$categories = $this->attr('categories');

		foreach ($keywords as $keyword) {
			if (preg_match('/(.+)_query_logic$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				$current_logic = array_get_index($_REQUEST, $keyword, array_get_index($fields[$matches[1]], 'word_logic'));
				$options = Array(
							'AND'	=> translate('sch_page_format_logic_and'),
							'OR'	=> translate('sch_page_format_logic_or'),
						   );
				ob_start();
					$this->registerFormField($keyword);
					combo_box($keyword, $options, FALSE, $current_logic);
					$replaces[$keyword] = ob_get_contents();
				ob_end_clean();
			} else if (preg_match('/(.+)_query_(.+)$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				$replaces[$keyword] = $this->getFieldReplacement($matches[1], $matches[2]);
			} else if (preg_match('/(.+)_query$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				$replaces[$keyword] = $this->getFieldReplacement($matches[1]);
			} else if (substr($keyword, -17) == '_category_chooser') {
				$cat_name = substr($keyword, 0, -17);
				$replaces[$keyword] = $this->getCategoryChooserReplacement($cat_name, $categories[$cat_name]);
			} else {
				$replaces[$keyword] = $this->getGeneralReplacement($keyword);
			}
		}

		return $replaces;

	}//end getInitialBodyReplacements()


	/**
	* Returns the keyword replacements for the specified format bodycopy
	*
	* @param string	$form_name	the name of the form to get the replacements for
	*
	* @return array
	* @access public
	*/
	function getFormFormatReplacements($form_name)
	{
		$bodycopy =& $this->getFormatBodycopy($form_name);
		if (is_null($bodycopy)) return Array();

		$fields = $this->attr('fields');
		$keywords = $bodycopy->getKeywords();

		$replacements = Array();
		foreach ($keywords as $keyword) {
			if (preg_match('/(.+)_query_terms$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				$replacements[$keyword] = $this->getSearchedFieldReplacement($matches[1]);
			} else if (preg_match('/(.+)_query$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				if ($form_name == 'sis_form') {
					$replacements[$keyword] = $this->getSisFieldReplacement($matches[1]);
				} else {
					$replacements[$keyword] = $this->getFieldReplacement($matches[1]);
				}
			} else {
				if ($form_name == 'sis_form' || $form_name == 'search_form') {
					if (substr($keyword, -17) == '_category_chooser') {
						$categories = $this->attr('categories');
						$cat_name = substr($keyword, 0, -17);
						$selected_option = array_get_index($_REQUEST, 'category_'.$cat_name, '');

						$replacements[$keyword] = $this->getCategoryChooserReplacement($cat_name, $categories[$cat_name], $selected_option);
					} else {
						$replacements[$keyword] = $this->getGeneralReplacement($keyword);
					}
				} else {
					$replacements[$keyword] = $this->getGeneralReplacement($keyword);
				}
			}
		}//end foreach

		return $replacements;

	}//end getFormFormatReplacements()


	/**
	* Returns the contents of the specified bodycopy with its keywords
	* replaced with the specified replacements
	*
	* @param string	$form_name		the name of the format to get the contents for
	* @param array	$replacements	the array of replacements to replace in the content
	*
	* @return string
	* @access public
	*/
	function getFormFormatContents($form_name, $replacements)
	{
		$bodycopy =& $this->getFormatBodycopy($form_name);
		if (is_null($bodycopy)) return '';

		$bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getFormFormatContents()


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
		// the search manager uses the type_code column in the search indexing table
		// to filter on asset types for performance reasons, so we don't need to
		// do any filtering
		return $todo;

	}//end filterAssetTypes()


	/**
	* Keyword-replaces for the results mode (wrapper function)
	*
	* @param array	&$results	the results returned
	* @param string	$bc_name	the name of the bodycopy to be printed (results | no_results)
	*
	* @return void
	* @access private
	*/
	function getResultsBodyReplacements(&$results, $bc_name='results')
	{
		$bodycopy =& $this->getBodycopy($bc_name);
		if (is_null($bodycopy)) {
			return Array();
		}

		$keywords = $bodycopy->getKeywords();

		// if we have %page_list_X% keywords in the array before the %result_list%
		// rip it off and put it at the end of the array or else it will not be
		// replace at all #4527 Matrix search page results has no output for page_list_X keywords
		$page_list_keywords = Array();
		foreach ($keywords as $index => $keyword) {
			if (preg_match('/page_list_([0-9]+)/', $keyword, $matches)) {
					$page_list_keywords[] = $keyword;
					unset($keywords[$index]);
				}
		}

		$keywords = array_merge($keywords, $page_list_keywords);

		// if we are to use the listing engine stuff then we need to change
		// to a getAssetList() style format
		if (!empty($results)) {
			$sort_info  = $this->getSortInfo();
			$asset_info = $this->sortAssetList($results, $sort_info);
		}

		$this->filterAssetList($asset_info);
		if (!is_array($asset_info)) $asset_info = Array();

		// Important!!! Missing these values in the overridden Listing Engine type of asset
		// will generate numerous notice errors when they try to group the result by asset type
		$this->_tmp['assets_info'] = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($asset_info));
		$this->_tmp['asset_urls'] = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_keys($asset_info));
		$this->_tmp['temp_results'] = $results;

		return $this->getLocalKeywordsReplacements($keywords, $asset_info);

	}//end getResultsBodyReplacements()


	/**
	* Keyword-replaces. Code extract from above to make into a
	* function so that we can use it directly with the apply_keyword_modifier()
	*
	* @param mixed	$keywords	the keywords we are looking te replacement for
	* @param array	$asset_info the asset_info of the results that we are going to process
	*
	* @return void
	* @access private
	*/
	function getLocalKeywordsReplacements($keywords, $asset_info=Array())
	{
		$replaces = Array();

		if (!$keywords) return $replaces;

		$mode = array_get_index($_REQUEST, 'mode', 'results');
		$keyword_as_string = FALSE;
		if (!is_array($keywords)) {
			$keywords = Array($keywords);
			$keyword_as_string = TRUE;
		}

		// get any extra keyword replacements from asset_listing
		if (empty($asset_info) && isset($this->_tmp['temp_results'])) {
			$sort_info  = $this->getSortInfo();
			$asset_info = $this->sortAssetList($this->_tmp['temp_results'], $sort_info);
		}
		$parent_replaces = $this->getContentsKeywordReplacements($keywords);

		// filter out inrelevent groups if group name must match query string feature is enabled.
		$group_name_filters = $this->_getSearchedQueries();

		if ($this->attr('group_by') == 'grouped') {
			$this->groupAssetsRecursively($this->attr('asset_grouping'), $asset_info, $asset_info, $group_name_filters);
			$this->sortGroups($this->attr('asset_grouping'), $asset_info, $asset_info);
		}

		$total_assets_count = $this->_arrayCountRecursive($asset_info);

		$listing_result_page = (int) array_get_index($_REQUEST, 'result_'.$this->id.'_result_page', 0);
		$search_result_page = (int) array_get_index($_REQUEST, 'current_result_page', 1);
		if ($listing_result_page === 0) {
			$current_result_page = $search_result_page;
		} else {
			$current_result_page = $listing_result_page;
		}//end if
		$results_per_page    = array_get_index($_REQUEST, 'results_per_page', $this->_getNumPerPage());
		$this->_tmp['start_element'] = ($current_result_page - 1) * $results_per_page;
		$queries = '';

		foreach ($_GET as $query_name => $query_value) {
			// skip some get vars that will be added later by the keyword replacements
			if (!in_array($query_name, Array('mode', 'current_result_page', 'results_per_page', 'result_'.$this->id.'_result_page'))) {
				if (is_array($query_value)) {
					foreach ($query_value as $value_index => $value_contents) {
						$queries .= '&'.urlencode($query_name).'['.urlencode($value_index).']='.urlencode($value_contents);
					}
				} else {
					$queries .= '&'.urlencode($query_name).'='.urlencode($query_value);
				}
			}
		}


		$asset_info = $this->getChunk($asset_info, $parent_replaces, $keywords, $current_result_page, $results_per_page);
		// For the 'select all' code
		$this->_analyseCheckAllRelations($asset_info);

		if ($results_per_page != 0) {
			$total_pages = isset($parent_replaces['total_pages']) ? $parent_replaces['total_pages'] : ceil(count($asset_info) / $results_per_page);
		} else {
			$total_pages = 1;
		}

		foreach ($keywords as $keyword) {
			$full_keyword = $keyword;
			$replacement = "%$full_keyword%";
			$keyword = parse_keyword($keyword, $modifiers);
			$apply_modifiers = TRUE;

			switch ($keyword) {
				case 'result_list':
					// print the asset list from the listing engine
					ob_start();
						$this->printAssetList($asset_info);
						$replacement = ob_get_contents();
					ob_end_clean();
				break;

				case 'initial_result_count' :
					$initial_results = $this->_tmp['initial_results'];
					$replacement = count($initial_results);
				break;

				case 'result_count':
					$replacement = $total_assets_count;
				break;

				case 'current_result_page':
					$replacement = min($current_result_page, $total_pages);
				break;

				case 'current_result_page_start':
					$start_record = ($current_result_page==1) ? $current_result_page : ((($current_result_page-1)*$results_per_page)+1);
					$replacement = $start_record;
				break;

				case 'current_result_page_end':
					$end_record = (($current_result_page) * $results_per_page);
					if ($end_record > $total_assets_count || $end_record == 0) {
						$end_record = $total_assets_count;
					}//end if
					$replacement = $end_record;

				break;

				case 'total_result_pages':
					$replacement = $total_pages;
				break;

				case 'result_page_index':
					$replacement = '';
					$current_page_format = $this->attr('current_page_format');
					$page_link_format = $this->attr('page_link_format');

					for ($i = 1; $i <= $total_pages; $i++) {
						if ($i == $current_result_page) {
							$replacement .= str_replace('%page_number%', $i, $current_page_format);
						} else {
							$href = ($this->attr('submit_to_page_url') ? $this->getURL() : current_url()).'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.$i.'&results_per_page='.$results_per_page.$queries);
							$replacement .= str_replace(Array('%page_number%', '%page_link%'), Array($i, $href), $page_link_format);
						}
					}
				break;

				case 'previous_result_page_link':
					if ($current_result_page <= 1) {
						$replacement = $this->attr('hide_no_href_links') ? '' : $this->attr('prev_page_text');
						break;
					}
					$replacement = '<a href="'.($this->attr('submit_to_page_url') ? $this->getURL() : current_url()).'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page - 1).'&results_per_page='.$results_per_page.$queries).'">'.$this->attr('prev_page_text').'</a>';
				break;

				case 'next_result_page_link':
					if ($current_result_page >= $total_pages) {
						$replacement = $this->attr('hide_no_href_links') ? '' : $this->attr('next_page_text');
						break;
					}
					$replacement = '<a href="'.($this->attr('submit_to_page_url') ? $this->getURL() : current_url()).'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page + 1).'&results_per_page='.$results_per_page.$queries).'">'.$this->attr('next_page_text').'</a>';
				break;

				case 'previous_result_page_href':
					if ($current_result_page <= 1) {
						$replacement = '';
						break;
					}
					$replacement = ($this->attr('submit_to_page_url') ? $this->getURL() : current_url()).'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page - 1).'&results_per_page='.$results_per_page.$queries);
				break;

				case 'next_result_page_href':
					if ($current_result_page >= $total_pages) {
						$replacement = '';
						break;
					}
					$replacement = ($this->attr('submit_to_page_url') ? $this->getURL() : current_url()).'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page + 1).'&results_per_page='.$results_per_page.$queries);
				break;

				case 'initial_layout':
					$replacement = $this->getBodycopyContents('initial', $this->getInitialBodyReplacements());
				break;

				default:
					$fields = $this->attr('fields');

					if (preg_match('/(.+)_query_logic$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						$current_logic = array_get_index($_REQUEST, $keyword, 'AND');
						$options = Array('AND' => translate('sch_page_format_logic_and'), 'OR' => translate('sch_page_format_logic_or'));
						ob_start();
							$this->registerFormField($keyword);
							combo_box($keyword, $options, FALSE, $current_logic);
							$replacement = ob_get_contents();
						ob_end_clean();

					} else if (preg_match('/(.+)_query_terms_(.+)$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						$replacement = $this->getSearchedFieldReplacement($matches[1], $matches[2]);

					} else if (preg_match('/(.+)_query_terms$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						$replacement = $this->getSearchedFieldReplacement($matches[1]);

					} else if (preg_match('/(.+)_query_synonym_search_message$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						if (!empty($this->_tmp['auto_synonym_search_used'])) {
							$replacement = $this->attr('auto_synonym_string');
						}

					} else if (preg_match('/(.+)_query_suggestions(_message)?$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						// spell-checking suggestions
						$field_name = $matches[1];
						if (isset($this->_tmp['search_suggestions'][$field_name])) {
							$queries_temp = str_replace('queries_'.$field_name.'_query='.urlencode($this->getSearchedFieldReplacement($field_name, '', FALSE)), 'queries_'.$field_name.'_query='.urlencode($this->_tmp['search_suggestions'][$field_name]), $queries);
							$spell_check_string = '<a href="'.($this->attr('submit_to_page_url') ? $this->getURL() : current_url()).'?mode='.$mode.'&current_result_page='.$current_result_page.'&results_per_page='.$results_per_page.$queries_temp.'">'.htmlspecialchars($this->_tmp['search_suggestions'][$field_name]).'</a>';
						} else {
							$spell_check_string = '';
						}

						if (!empty($spell_check_string)) {
							if (substr($keyword, -8) == '_message') {
								$message = $this->attr('spell_check_string');
								$replacement = str_replace('%suggestions%', $spell_check_string, $message);
							} else {
								$replacement = $spell_check_string;
							}
						}

					} else if (preg_match('/(.+)_query_synonyms(_message)?$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						// synonym searching
						$field_name = $matches[1];
						$synonym_string = Array();
						if (isset($this->_tmp['search_synonyms'])) {
							foreach (array_get_index($this->_tmp['search_synonyms'], $field_name, Array()) as $word => $synonyms) {
								foreach ($synonyms as $synonym) {
									$queries_temp = str_replace('queries_'.$field_name.'_query='.urlencode($this->getSearchedFieldReplacement($field_name, '', FALSE)), 'queries_'.$field_name.'_query='.urlencode($synonym), $queries);
									$synonym_string[] = '<a href="'.($this->attr('submit_to_page_url') ? $this->getURL() : current_url()).'?mode='.$mode.'&current_result_page='.$current_result_page.'&results_per_page='.$results_per_page.$queries_temp.'">'.htmlspecialchars($synonym).'</a>';
								}
							}
						}
						$synonym_string = implode(', ', $synonym_string);

						if (!empty($synonym_string)) {
							if (substr($keyword, -8) == '_message') {
								$message = $this->attr('synonym_string');
								$replacement = str_replace('%suggestions%', $synonym_string, $message);
							} else {
								$replacement = $synonym_string;
							}
						}

					} else if (preg_match('/(.+)_query_(.+)$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						$replacement = $this->getFieldReplacement($matches[1], $matches[2]);

					} else if (preg_match('/(.+)_query$/', $keyword, $matches) && array_key_exists($matches[1], $fields)) {

						$replacement = $this->getFieldReplacement($matches[1]);

					} else if (substr($keyword, -7) == '_linked') {

						if ($keyword == 'category_list_linked') {
							// printing the list of seach categories
							$categories = $this->attr('categories');
							if (!empty($categories)) {
								$query_string_vars = Array(
														'mode'					=> $mode,
														'current_result_page'	=> 1,
													 );
								ob_start();
									?>
									<div id="search_category_list">
										<?php
										foreach ($categories as $cat_name => $cat_data) {
											?>
											<ul>
												<?php
												$current_option = array_get_index($_GET, 'category_'.$cat_name, '');
												$cat_data['options'] = Array('' => Array('full_name' => $cat_data['unselected_text'])) + $cat_data['options'];
												foreach ($cat_data['options'] as $key => $data) {
													$query_string_vars['category_'.$cat_name] = $key;
													$url = htmlspecialchars(replace_query_string_vars($query_string_vars));
													?>
													<li<?php echo ($current_option == $key) ? ' id="current"' : ''; ?>>
														<a href="<?php echo $url; ?>">
															<?php echo $data['full_name']; ?>
														</a>
													</li>
													<?php
												}
												unset($query_string_vars['category_'.$cat_name]);
												?>
											</ul>
											<?php
										}
										?>
									</div>
									<?php
									$replacement = ob_get_contents();
								ob_end_clean();
							} else {
								$replacement = '';
							}

						} else if (substr($keyword, -13) == '_query_linked') {

							$options = Array();
							$not_selected_text = '';

							$query_var = substr($keyword, 0, (strlen($keyword) - 13));
							$fields = $this->attr('fields');

							if ($fields[$query_var]['type'] == 'metadata') {

								$assetid = $fields[$query_var]['params']['assetid'];
								$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								$params = $asset->vars;
								// metadata dropdown selection
								if (isset($params['select_options'])) {
									$options = $params['select_options']['value'];
								}

							} else if ($fields[$query_var]['type'] == 'asset_attrib') {

								$search_styles = $this->attr('search_styles');
								$not_selected_text = '';

								$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($fields[$query_var]['params']['attrid']);

								if ($attribute->type() == 'selection') {
									if (empty($attribute->_params['multiple'])) {
										$options = $attribute->_params['options'];
									}
									$not_selected_text = array_get_index($search_styles[$attribute->id], 'empty_text');
								} else if ($attribute->type() == 'boolean') {
									if (isset($search_styles[$attribute->id])) {
										$options = Array(
													'0'	=> $search_styles[$attribute->id]['false_text'],
													'1'	=> $search_styles[$attribute->id]['true_text'],
												   );
									} else {
										$options = Array('0' => translate('false'), '1' => translate('true'));
									}
									$not_selected_text = array_get_index($search_styles[$attribute->id], 'empty_text');
								}
							}//end if an attribute

							if (!empty($options)) {
								$query_key = 'queries_'.$query_var.'_query';
								$current_value = (string) array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $query_var, '');

								$base_url = $_SERVER['PHP_SELF'].'?'.$_SERVER['QUERY_STRING'];
								$base_url = preg_replace('|([&?])mode=([^&]+)|', '\\1mode='.$mode, $base_url);
								$base_url = preg_replace('|&'.$query_key.'=([^&]+)?|', '', $base_url);
								$base_url = preg_replace('|&current_result_page=([^&]+)?|', '&current_result_page=1', $base_url);
								$base_url .= '&'.$query_key.'=';

								if ($not_selected_text == '') {
									$not_selected_text = translate('no_selection');
								}
								ob_start();
									?>
									<div id="search_<?php echo $query_var; ?>_query_list">
										<ul>
											<li<?php echo ($current_value == '') ? ' id="current"' : ''; ?>>
												<a href="<?php echo $base_url; ?>"><?php echo $not_selected_text; ?></a>
											</li>
										<?php
										foreach ($options as $key => $value) {
											$key = (string) $key;
											?>
											<li<?php echo ($current_value == $key) ? ' id="current"' : ''; ?>>
												<a href="<?php echo $base_url.$key; ?>"><?php echo $value; ?></a>
											</li>
											<?php
										}
										?>
										</ul>
									</div>
									<?php
									$replacement = ob_get_contents();
								ob_end_clean();
							}//end if
						}//end if category links

					} else {

						// use the parent's replacement if it exists, otherwise revert to the default
						if (isset($parent_replaces[$keyword])) {
							$replacement = $parent_replaces[$keyword];
						} else {
							// Avoid messing with globals and other keywords in the fallback to Asset::getKeywordReplacement() by passing the full keyword.
							$replacement = $this->getGeneralReplacement($full_keyword);
							$apply_modifiers = FALSE;
						}

					}
				break;
			}//end switch

			if ($apply_modifiers && $replacement != "%$full_keyword%"){
				// we passing in the 'call_fns' argument below to make sure we use this current function if in case
				// there are any  nested keywords that needs to be replaced by the apply_keyword_modifiers function
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id, 'call_fns' => Array('getLocalKeywordsReplacements')));
			}

			$replaces[$full_keyword] = $replacement;

		}//end foreach

		if ($keyword_as_string) {
			$replaces = $replaces[$keywords[0]];
		}

		return $replaces;

	}//end getLocalKeywordsReplacements()


	/**
	* Keyword-replaces for the result element
	*
	* @param int	$assetid	the asset ID of the result element
	*
	* @return array
	* @access public
	*/
	function getResultElementBodyReplacements($assetid)
	{
		$bodycopy =& $this->getBodycopy('result_element');
		if (is_null($bodycopy)) {
			return Array();
		}
		$keywords = $bodycopy->getKeywords();

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$asset = $am->getAsset($assetid);

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$replaces = $mm->generateKeywordReplacements($asset, $keywords, FALSE);

		$metadata_keywords = Array();
		foreach ($keywords as $key) {
			if (substr($key, 0, 15) == 'asset_metadata_') {
				$metadata_keywords[$key] = substr($key, 15);
			}
		}
		if (!empty($metadata_keywords)) {
			$metadata_values = $mm->getMetadataFieldValues($assetid, array_values($metadata_keywords));
			foreach ($metadata_keywords as $key => $field) {
				if (isset($metadata_values[$field])) {
					$replaces[$key] = $metadata_values[$field];
				}
			}
		}

		$asset_keywords = $asset->getAvailableKeywords();
		foreach ($asset_keywords as $key => $description) {
			if (in_array($key, $keywords)) {
				$replaces[$key] = $asset->getKeywordReplacement($key);
			}
		}

		foreach ($keywords as $keyword) {

			$replacement =& $replaces[$keyword];
			switch ($keyword) {
				case 'asset_id':
					$replacement = $assetid;
				break;

				case 'asset_name_linked':
					$replacement = '<a href="'.$asset->getUrl().'">'.$asset->name.'</a>';
				break;

				case 'asset_short_name_linked':
					$replacement = '<a href="'.$asset->getUrl().'">'.$asset->short_name.'</a>';
				break;

				case 'asset_lineage':
				case 'asset_lineage_linked':
					$replacement = '';
					$first = TRUE;
					$found_root = FALSE;
					$nurl = $asset->getUrl();
					$unrl = strip_url($nurl);
					$nurl = stristr($nurl,'/');
					$nurl = substr($nurl,2);
					$parent_asset_ids = $am->getLineageFromURL(NULL,$nurl);
					// searching for the root asset
					$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'root');
					$root_assetid = empty($root_link) ? 0 : $root_link['minorid'];
					foreach ($parent_asset_ids as $parent_asset_id) {
						if ($parent_asset_id == $root_assetid) {
							$found_root = TRUE;
						}
						if (!$found_root) continue;
						if (!$first) {
							$replacement .= $this->attr('lineage_seperator');
						}
						$first = FALSE;
						$ancestor_asset = $am->getAsset($parent_asset_id);
						if ($keyword == 'asset_lineage_linked') {
							$replacement .= '<a href="'.$ancestor_asset->getUrl().'">';
						}
						$replacement .= $ancestor_asset->short_name;
						if ($keyword == 'asset_lineage_linked') {
							$replacement .= '</a>';
						}
						$am->forgetAsset($ancestor_asset);
					}
				break;
			}//end switch
		}//end foreach

		return $replaces;

	}//end getResultElementBodyReplacements()


	/**
	* Returns a list of extra keywords that should be replaced for a particular
	* listing asset
	*
	* This allows search page, for instance, to define its result number and
	* score fields so they don't blank out when keywords are replaced in listing
	* engine.
	*
	* @return array
	* @access public
	*/
	function getExtendedAssetKeywordReplacements()
	{
		// We are only replacing them with their own replacement so they can be
		// cached out like this, then score and result number can be added later
		return Array(
				'result_number'	=> '%result_number%',
				'score'			=> '%score%',
			   );

	}//end getExtendedAssetKeywordReplacements()


	/**
	*
	* Add date range restriction to search results by adding a date range search field with
	* special name 'matrix_search_page_date_range'
	*
	* @param array	$fields	Details fields from fields attr
	* @param array	$queries Details of search queries
	*
	* @return void
	* @access private
	*/
	function _addDateRangeRestriction (&$fields, &$queries)
	{
		$restrict_date_field = $this->attr('restrict_date_field');
		$to = $this->getAttribute('restrict_date_range_to');
		$from = $this->getAttribute('restrict_date_range_from');

		// return if restriction is not set
		if(empty($restrict_date_field) || ( $to->value === '---------- --:--:--' && $from->value === '---------- --:--:--')) {
			return;
		}
		$from = $from->getISO8601();
		$from_date = iso8601_date_component($from);
		if (!empty($from_date)) {
			// invalid FROM date?
			if (!preg_match_all('|^\d{4}-\d{2}-\d{2}$|', $from_date, $matches)) {
				trigger_localised_error('CMS0053', E_USER_WARNING, $from_date);
				return;
			}

			$from_time = iso8601_time_component($from);
			if (empty($from_time)) $from_time = '00:00:00';

			// invalid FROM time?
			if (!preg_match_all('|^\d{2}:\d{2}:\d{2}$|', $from_time, $matches)) {
				trigger_localised_error('CMS0054', E_USER_WARNING, $from_time);
				return;
			}
			$restrict_date_range_from = $from_date.' '.$from_time;
		}

		$to = $to->getISO8601();
		$to_date = iso8601_date_component($to);
		if (!empty($to_date)) {
			// invalid TO date?
			if (!preg_match_all('|^\d{4}-\d{2}-\d{2}$|', $to_date, $matches)) {
				trigger_localised_error('CMS0055', E_USER_WARNING, $to_date);
				return;
			}

			$to_time = iso8601_time_component($to);
			if (empty($to_time)) $to_time = '23:59:59';

			// invalid TO time?
			if (!preg_match_all('|^\d{2}:\d{2}:\d{2}$|', $to_time, $matches)) {
				trigger_localised_error('CMS0056', E_USER_WARNING, $to_time);
				return;
			}
			$restrict_date_range_to = $to_date.' '.$to_time;
		}

			$content = Array();
			$content['name'] = 'matrix_search_page_date_range';
			$content['word_logic'] = 'AND';
			$content['specific_field_logic'] = 'AND';
			$content['data_sources'] = Array (
											0 => Array (
													'type' => 'standard',
													'params' => Array ('field' => $restrict_date_field)
												)
									);
			$fields['matrix_search_page_date_range'] = $content;
			$queries['matrix_search_page_date_range'] = Array (
													'from' => isset($restrict_date_range_from) ? $restrict_date_range_from : '---------- --:--:--',
													'to' => isset($restrict_date_range_to) ? $restrict_date_range_to : '---------- --:--:--'
												);
	}//end _addDateRangeRestriction()



	/**
	* Processes a search request
	*
	* Note that all search queries are constructed and run in the Search Manager
	* From the posted variables, it formulates search info and returns results
	*
	* @param array	$search	The search variables used for stored searches
	*
	* @return array
	* @access public
	* @see Search_Manager:processSearch()
	*/
	function processSearch($search=Array())
	{
		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

		$fields = $this->attr('fields');
		$queries = $this->_getSearchedQueries();

		// Add date range restriction as a search field and query.
		$this->_addDateRangeRestriction($fields, $queries);

		// If the search params were passed, get them and use them, if the search queries are empty
		if (!empty($search) || count($search)>0) {
			foreach ($search as $search_key => $search_value) {
				if (!isset($_REQUEST['queries_'.$search_key.'_query_posted'])) {
					if (!isset($queries[$search_key]) || empty($queries[$search_key])) {
						$queries[$search_key] = $search_value;
					}//end if
				}//end if
			}//end foreach
		}//end if

		// check if we are allowing the keywords to be replaced in user input
		if($this->attr('disable_keyword_replacements')) {
			$queries = $this->_sanitizeQueries($queries);
		}

		// save the search terms into the session so that we can use them for
		// search within search results
		$_SESSION['SQ_LAST_SEARCH'][$this->id] = $queries;

		// get the basic search settings based on the current category (if any)
		$search_info = $this->populateBaseSearchInfo();
		if (empty($search_info['fields'])) {
			$search_info['fields'] = Array();
		}

		$query_fields_exist = FALSE;

		foreach ($queries as $field_name => $value) {

			if (empty($value)) continue;

			$empty_numeric = Array(
								'upper'	=> NULL,
								'lower'	=> NULL,
							 );
			if ($value == $empty_numeric) continue;

			$empty_dates = Array(
							'from'	=> '---------- --:--:--',
							'to'	=> '---------- --:--:--',
						   );
			if ($value == $empty_dates) continue;

			if (count($fields[$field_name]['data_sources']) == 0) {
				trigger_localised_error('SCH0028', E_USER_NOTICE, $field_name);
				continue;
			}

			if (!$this->_hasMixedFieldTypes($fields[$field_name]) && $fields[$field_name]['data_sources'][0]['type'] == 'asset_attrib') {
				$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($fields[$field_name]['data_sources'][0]['params']['attrid']);
				if (!is_null($attribute) && $attribute->type() == 'boolean') {
					// we need to convert the value into a word representation,
					// since indexing service will not operate just 0 or 1
					// do not localise this!!
					if ($value == '0') {
						$value = 'false';
					} else if ($value == '1') {
						$value = 'true';
					}
				}//end if
			}//end if
			$search_info['fields'][$field_name] = $fields[$field_name];
			$search_info['fields'][$field_name]['words'] = $value;
			$search_info['fields'][$field_name]['exclude_field'] = array_get_index($fields[$field_name], 'exclude_field', FALSE);

			// If there is aleast one "regular" search field, we have the query fields
			if (!$search_info['fields'][$field_name]['exclude_field']) {
				$query_fields_exist = TRUE;
			}

			$submitted_word_logic = strtoupper(array_get_index($_REQUEST, $field_name.'_query_logic', ''));
			if (in_array($submitted_word_logic, Array('AND', 'OR'))) {
				$search_info['fields'][$field_name]['word_logic'] = $submitted_word_logic;
			}

		}//end foreach fields

		// spell-check suggestions
		if (!empty($search_info['fields'])) {
			foreach ($search_info['fields'] as $field_name => $field_content) {
				$words = $field_content['words'];
				if (!is_array($words)) $words = explode(' ', $words);
				if ($sm->spellCheckAvailable()) {
					// suggestion 'string' - this will be imploded later with spaces
					$suggestion_string = Array();
					$needs_suggestions = FALSE;

					// loop over each word
					foreach ($words as $word) {

						// figure out quote positioning
						preg_match_all('/"/', $word, $raw_positions, PREG_OFFSET_CAPTURE);
						$quote_positions = Array();
						if (!empty($raw_positions)) {
							foreach ($raw_positions[0] as $position) {
								$quote_positions[] = $position[1];
							}
						}

						$suggestions = $sm->spellCheckWord($word);
						if (!empty($suggestions)) {
							$needs_suggestions = TRUE;
							$suggestion        = $suggestions[0];

							foreach ($quote_positions as $position) {
								// we can easily re-add quotes to the beginning and
								// end of a word, but in the middle is a bit tricky
								if ($position == 0) {
									$suggestion = '"'.$suggestion;
								} else if ($position == strlen($word) - 1) {
									$suggestion = $suggestion.'"';
								}
							}

							$suggestion_string[] = $suggestion;
						} else {
							$suggestion_string[] = $word;
						}
					}

					if ($needs_suggestions) {
						$this->_tmp['search_suggestions'][$field_name] = implode(' ', $suggestion_string);
					}
				}//end if spell check available

				// thesaurus synonym searching
				$t_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'asset', FALSE, 'thesaurus');

				$synonyms =& $this->_tmp['search_synonyms'];

				if (!empty($t_link)) {
					$thesaurus = $GLOBALS['SQ_SYSTEM']->am->getAsset($t_link['minorid']);
					$synonyms[$field_name] = Array();

					// loop over each word, find synonyms
					foreach ($words as $word) {
						if (!isset($synonyms[$field_name][$word])) {
							$synonyms[$field_name][$word] = $thesaurus->getSynonymsForTerm($word);
						}
					}
				}//end thesaurus exists
			}//end foreach
		}//end if

		// Metadata Hierarchy
		// requires us to go back up and down the tree we have to add the extra looking here
		if (isset($search_info['fields'])) {
			foreach ($search_info['fields'] as $fieldname => $field) {
				foreach ($field['data_sources'] as $data_src) {
					if (isset($data_src['type']) && $data_src['type'] == 'metadata') {
						if (isset($data_src['params']['assetid'])) {
							$m_field_id = $data_src['params']['assetid'];
						}
						if ($m_field_id > 0) {
							$m_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($m_field_id);
							if (!is_null($m_field)) {
								if ($m_field->type() == 'metadata_field_hierarchy') {
									$options = $m_field->attr('hierarchy_options');
									$depth = $m_field->attr('hier_depth');
									$edit_fns = $m_field->getEditFns();
									if (is_array($search_info['fields'][$fieldname]['words'])) {
										// If multiple entries submitted, they arrive as an array, so convert to string
										$word_list = implode(' ', $search_info['fields'][$fieldname]['words']);
										$search_info['fields'][$fieldname]['words'] = $word_list;
									}
									$search_info['fields'][$fieldname]['words'] = $edit_fns->getHierarchyTree($search_info['fields'][$fieldname]['words'], $options, ' ');
									$search_info['fields'][$fieldname]['word_logic'] = 'OR';
								}//end if
							}//end if
						}//end if
					}//end if
				}//end foreach()

				// Seperate "exclude" fields from regular "search" fields
				if ($field['exclude_field']) {
					$search_info['exclude'][$fieldname] = $search_info['fields'][$fieldname];
					unset($search_info['fields'][$fieldname]);
				}//end if

			}//end foreach()
		}//end if

		if ($query_fields_exist) {

			// If date range restriction is the only query field set, it's actually an empty search.
			// Return empty array if empty search is not allowed.
			if(!$this->attr('allow_empty_search') && count($search_info['fields']) == 1 && isset($search_info['fields']['matrix_search_page_date_range'])){
				return Array();
			}

			$auto_ss_field = $this->attr('auto_synonym_search_field');
			$search_results = $sm->processSearch($search_info);

			if (($this->attr('enable_multi_word_synonym_lookups')) && (!empty($auto_ss_field))) {
				$search_results = $this->getMultiWordSynonymSearchResults($search_results, $search_info, $auto_ss_field, $thesaurus);
			}

			// If there are no results, check to see whether there is an auto
			// synonym search set up, and if so, use that
			if (empty($search_results)) {
				if (!empty($auto_ss_field) && isset($search_info['fields'][$auto_ss_field])) {
					// Search with a copy of the original search info, because
					// we don't want to interfere with the original search
					$syn_search_info = $search_info;
					$this->_tmp['auto_synonym_search_used'] = TRUE;

					$search_field = $search_info['fields'][$auto_ss_field];
					$words = $search_field['words'];
					if (!is_array($words)) $words = explode(' ', $words);

					// automatically search by synonyms
					$synonym_words = Array();

					// Note that even if the original query had word logic of
					// AND, we will search for all synonyms found using OR.
					// The next logical step would be to provide some way where
					// each word's synonyms are ANDed together, but this would
					// require a large overhaul to get this to work in
					// PostgreSQL
					foreach ($words as $word) {
						$synonym_words += $synonyms[$auto_ss_field][$word];
					}

					if (!empty($synonym_words)) {
						$syn_search_info['fields'][$auto_ss_field]['words'] = implode(' ', $synonym_words);
						$search_results = $sm->processSearch($syn_search_info);
					}
				}

			}//end if

			return $search_results;

		} else if ($this->attr('allow_empty_search')) {
			return $sm->processBasicSearch($search_info);
		} else {
			return Array();
		}

	}//end processSearch()


	/*
	* Returns all combinations of words from a string, of length 2 to "string length" words.
	* "Combinations" as in the contiguous mathematical sense.
	*
	* For example:
	*	supplied words  = 'high', 'climbing', 'walls'
	*	combinations    = 'high climbing', 'climbing walls', 'high climbing walls'
	*
	* @param array	$words	The search words to iterate (in order)
	*
	* @return array
	* @access private
	*/
	private function _getWordCombinations(Array $words)
	{
		$word_combinations = Array();

		// Only return combinations if we have at least three words, otherwise return a string of what was supplied
		$num_words = count($words);
		if ($num_words < 3) {
			return Array(implode(' ', $words));
		}

		// Chunk is back. Grab "string length" down to two-word combinations at each step
		// This is better illustrated with coloured pencils
		for ($chunk_size=$num_words; $chunk_size >= 2; $chunk_size--)
		{
			for ($start_index=0; $start_index <= $num_words - $chunk_size; $start_index++)
			{
				$combination = '';
				for ($n=0; $n < $chunk_size; $n++)
				{
					$word_index = $start_index + $n;
					$combination .= $words[$word_index].' ';
				}

				// Add combination to our list and trim trailing space
				$word_combinations[] = rtrim($combination);
			}
		}

		return $word_combinations;

	}//end _getWordCombinations()


	/**
	* Get an array of all search terms that have been submitted
	*
	* Also adjusts fields if we are performing a search within search by concating them
	* with the terms used for the last search
	*
	* @return array
	* @access private
	*/
	function _getSearchedQueries()
	{
		if (isset($this->_tmp['searched_queries'])) {
			return $this->_tmp['searched_queries'];
		}

		$fields = $this->attr('fields');
		$mode = array_get_index($_REQUEST, 'mode', 'results');
		if ($mode == '') $mode = 'results';
		$suffix = ($mode == 'results') ? '_query' : '_query_sis';
		$queries = Array();
		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		foreach ($fields as $field_name => $field_details) {

			$queries[$field_name] = '';
			$query_var = 'queries_'.$field_name.$suffix;
			// Bug #3002
			// If there is a period (.) in the field name, REQUEST var changes it to a underscore (_), so when
			// setting the query_var string, we need to look for underscores and not periods in the REQUEST var
			// It should be noted that query_var is used elsewhere too.
			$clean_query_var = str_replace('.', '_', $query_var);

			if (!empty($field_details['data_sources']) && !$this->_hasMixedFieldTypes($field_details)) {
				$data_source = $field_details['data_sources'][0];
				switch ($data_source['type']) {
					case 'include_all' :
						$queries[$field_name] = array_get_index($_REQUEST, $clean_query_var, '');
					break;

					case 'standard' :
						if (isset($sm->standard_date_fields[$data_source['params']['field']])) {
							$queries[$field_name] = $this->_processDateRange($query_var);
						} else if (isset($sm->standard_text_fields[$data_source['params']['field']])) {
							$queries[$field_name] = array_get_index($_REQUEST, $clean_query_var, '');
						}
					break;

					case 'asset_attrib' :
						$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);
						if (!is_null($attribute)) {
							if ($attribute->type() == 'datetime') {
								$queries[$field_name] = $this->_processDateRange($query_var);
							} else if (($attribute->type() == 'int') || ($attribute->type() == 'float')) {
								$queries[$field_name] = Array();

								switch (array_get_index($data_source['params'], 'numeric_search', 'exact')) {
									case 'exact':
										$query = array_get_index($_REQUEST, $clean_query_var, NULL);
										if (!is_null($query)) {
											$queries[$field_name]['lower'] = $query;
											$queries[$field_name]['upper'] = $queries[$field_name]['lower'];
										}
									break;

									case 'range':
										$query_lower = array_get_index($_REQUEST, $clean_query_var.'_lower', NULL);
										$query_upper = array_get_index($_REQUEST, $clean_query_var.'_upper', NULL);
										if (!is_null($query_lower) || !is_null($query_upper)) {
											$queries[$field_name]['lower'] = ($query_lower == '') ? NULL : $query_lower;
											$queries[$field_name]['upper'] = ($query_upper == '') ? NULL : $query_upper;
										}
									break;

									case 'custom':
										$key = array_get_index($_REQUEST, $clean_query_var, NULL);

										if (!is_null($key)) {
											$custom_range = array_get_index($data_source['params']['custom_num_search'], $key, Array());
											$queries[$field_name]['lower'] = ($custom_range['min'] == '') ? NULL : $custom_range['min'];
											$queries[$field_name]['upper'] = ($custom_range['max'] == '') ? NULL : $custom_range['max'];
										}
									break;
								}
							} else {
								if ($attribute->type() == 'boolean' || $attribute->type() == 'selection') {
									$attribute->_params['allow_empty'] = TRUE;
								}
								$attribute->process($query_var, FALSE);
								$queries[$field_name] = trim($attribute->value);
								if ($attribute->type() == 'selection' && !empty($attribute->_params['multiple'])) {
									$queries[$field_name] = array_keys($attribute->getValueArray());
								}
							}
						}//end if
					break;

					case 'metadata' :
						if (empty($data_source['params']['assetid'])) {
							trigger_localised_error('SCH0027', E_USER_NOTICE, $field_name);
							$queries[$field_name] = '';
							break;
						}
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['params']['assetid']);
						if (!is_null($asset)) {
							$params = $asset->vars;
							switch (get_class_lower($asset)) {
								case 'metadata_field_date' :
									$queries[$field_name] = $this->_processDateRange($query_var);
								break;

								default :
									$queries[$field_name] = array_get_index($_REQUEST, $clean_query_var, '');
								break;
							}
						}
					break;
				}//end switch query type
			} else {
				$queries[$field_name] = array_get_index($_REQUEST, $clean_query_var, '');
			}

		}//end foreach fields

		if ($mode == 'sis') {
			$last_search = $_SESSION['SQ_LAST_SEARCH'][$this->id];

			foreach ($queries as $field_name => $value) {

				if (is_scalar($value)) $value = trim($value);
				if (empty($value)) continue;
				$empty_dates = Array(
								'from'	=> '---------- --:--:--',
								'to'	=> '---------- --:--:--',
							   );
				if ($value == $empty_dates) continue;

				if (!isset($last_search[$field_name]) || $last_search[$field_name] == '') {
					$last_search[$field_name] = $value;
				} else {
					// trim the current value before we append more words to it
					if (is_scalar($last_search[$field_name])) {
						$last_search[$field_name] = trim($last_search[$field_name]);
					}
					if (!is_array($value)) {
						$last_search[$field_name] .= ' '.$value;
					} else if (isset($value['from']) || isset($value['to'])) {
						// this is a date field
						if (isset($value['from'])) {
							$last_search[$field_name]['from'] = $value['from'];
						}
						if (isset($value['to'])) {
							$last_search[$field_name]['to']   = $value['to'];
						}
					} else {
						$last_search[$field_name] = array_merge($value, $last_search[$field_name]);
					}
				}
			}//end foreach

			$queries = $last_search;
		}//end if

		// if it seems that nothing is submitted, check if there was a previous query
		// and use its details
		if (empty($queries) && !empty($this->_tmp['potential_searched_queries'])) {
			$queries = $this->_tmp['potential_searched_queries'];
		}

		return $queries;

	}//end _getSearchedQueries()


	/**
	* Get the basic search settings based on the current category
	*
	* @return boolean
	* @access private
	*/
	function populateBaseSearchInfo()
	{
		$prefix = $this->getPrefix();

		// first, see if a category has been supplied
		$categories = $this->attr('categories');
		$active_cats = Array();
		foreach ($categories as $set_name => $cat_data) {
			if (!empty($_GET['category_'.$set_name])) {
				if (isset($cat_data['options'][$_GET['category_'.$set_name]])) {
					$code_name = $_GET['category_'.$set_name];
					$active_cats[$set_name.'_'.$code_name] = $cat_data['options'][$code_name];
				}
			}
		}

		$search_roots = Array();
		$statuses_customised = FALSE;
		$root_logic = 'AND'; // if there are no roots from categories this will get changed below
		if (!empty($active_cats)) {
			include_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$search_statuses = array_keys(get_status_names());
			foreach ($active_cats as $set_option_name => $cat) {
				if (isset($cat['statuses'])) {
					$statuses_customised = TRUE;
					$search_statuses = array_intersect($search_statuses, $cat['statuses']);
				}
				if (isset($cat['root_assetid'])) {
					$link_value = 'root_'.$set_option_name;
					$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, $link_value);
					$root_id = array_get_index($root_link, 'minorid');
					if ($root_id) $search_roots[] = $root_id;
				}
			}
		}

		if (!$statuses_customised) {
			// use default status choices
			$search_statuses = $this->attr('statuses');
		}

		if (empty($search_roots)) {
			// use default search roots because no categories chosen or not customised
			$search_roots =  $this->getRootNodes();
			$root_logic = 'OR';
			// if no default root nodes were specified, we search everything
			if (empty($search_roots)) $search_roots = 0;
		}

		$search_info = Array();
		$search_info['statuses'] = $search_statuses;
		$search_info['roots'] = $search_roots;
		$search_info['root_logic'] = $root_logic;
		$search_info['field_logic'] = $this->attr('logic');
		$search_info['requester'] = $this->id;
		$search_info['asset_types'] = $this->attr('types');
		$search_info['search_as_public_user'] = $this->attr('search_as_public_user');
		$search_info['no_roles_check'] = $this->attr('no_roles_check');
		$search_info['no_group_access_check'] = $this->attr('no_group_access_check');

		return $search_info;

	}//end populateBaseSearchInfo()


	/**
	* Returns the link for the format folder
	*
	* @return array
	* @access public
	*/
	function getFormatFolderLink()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'folder', TRUE, 'major', 'format_folder');
		return (!empty($link)) ? $link[0] : Array();

	}//end getFormatFolderLink()


	/**
	* Returns the format folder where the format bodycopies live
	*
	* @return object
	* @access public
	*/
	function &getFormatFolder()
	{
		$folder = NULL;

		$link = $this->getFormatFolderLink();
		if (!empty($link)) {
			$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		}

		return $folder;

	}//end getFormatFolder()


	/**
	* Returns the link for a bodycopy with the specified link value
	*
	* @param string	$link_value	the link value of the link of the wanted bodycopy
	*
	* @return array
	* @access public
	*/
	function getFormatBodycopyLink($link_value)
	{
		$format_folder =& $this->getFormatFolder();
		if (is_null($format_folder)) return Array();

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($format_folder->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', TRUE, 'major', $link_value);
		return (!empty($link)) ? $link[0] : Array();

	}//end getFormatBodycopyLink()


	/**
	* Returns the format bodycopy with the specifed link value
	*
	* If the bodycopy does not exist, or it have a link type 3, NULL is returned
	* as either no format has been customised, or the format feature has been disbled
	*
	* @param string	$link_value	the link value of the wanted bodycopy
	*
	* @return object
	* @access public
	*/
	function &getFormatBodycopy($link_value)
	{
		$bodycopy = NULL;

		$bodycopy_link = $this->getFormatBodycopyLink($link_value);
		if (!empty($bodycopy_link)) {
			$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		}

		return $bodycopy;

	}//end getFormatBodycopy()


	/**
	* Prints the individual asset in the asset list
	*
	* This is being overridden to cache the results of each asset, and add
	* the score and result number keyword replacements
	*
	* @param string	$assetid		The assetid of the asset we are printing
	* @param int	$list_position	Asset's position in the list
	* @param int	$num_items		the number of items in the current list (required by Listing Engine)
	*
	* @return void
	* @access protected
	*/
	function _printAsset($assetid, $list_position, $num_items=0)
	{
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$cache_key = $assetid.'-ctx'.$contextid;

		// the $this->_tmp[] variables are set in getResultsBodyReplacements()
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$element_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);

		if ($element_contents === FALSE) {
			// keyword replacements for %result_number% and %score% are now handled
			// by the "extended asset" keywords replacements defined in getExtendedAssetKeywordReplacements()

			ob_start();
			parent::_printAsset($assetid, $list_position, $num_items);

			$element_contents = ob_get_contents();
			ob_end_clean();
			$cm->saveToCache($this->id, $this->type(), $cache_key, $element_contents);
		}

		// summon the scores at this point. and work with new DAL result format
		$score =& $this->_tmp['search_results'];
		$score = (isset($score['type_code'])) ? $score['type_code'] : $score;
		$weight = (is_array($score[$assetid])) ? $score[$assetid][0]['weight'] : $score[$assetid];

		$element_replacements = Array(
									'score'			=> $weight,
									'result_number'	=> $this->_tmp['start_element'] + $list_position,

								);

		replace_keywords($element_contents, $element_replacements);

		echo $element_contents;

	}//end _printAsset()


	/**
	* Get the value of a query field
	*
	* Looks at both the query vars and session data
	*
	* @param string	$field_name	field name
	*
	* @return string
	* @access public
	*/
	function getQueryFieldValue($field_name='')
	{
		$field_value = '';

		if (empty($field_name)) return '';

		$fields = $this->attr('fields');
		$field_data = array_get_index($fields, $field_name);
		if (empty($field_data)) return '';

		$query_var = 'queries_'.$field_name.'_query';
		$field_value = array_get_index($_REQUEST, $query_var,'');

		if (empty($field_value) && isset($_SESSION['SQ_LAST_SEARCH'][$this->id][$field_name])) {
			$field_value = $_SESSION['SQ_LAST_SEARCH'][$this->id][$field_name];
		}

		return $field_value;

	}//end getQueryFieldValue()


//--        KEYWORD DESCRIPTION        --//


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
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');

		// if there is a search in search bodycopy add it to the bodycopy list
		// it will need some of the initial page keywords
		$sis_bodycopy_link    = $this->getFormatBodycopyLink('sis_form');
		if (!empty($sis_bodycopy_link)) {
			$bodycopy_links[] = $sis_bodycopy_link;
		}

		// if there is a search in search bodycopy add it to the bodycopy list
		// it will need some of the initial page keywords
		$search_bodycopy_link    = $this->getFormatBodycopyLink('search_form');
		if (!empty($search_bodycopy_link)) {
			$bodycopy_links[] = $search_bodycopy_link;
		}

		$bodycopies   = Array();
		$type_formats = Array();
		$keywords     = Array();

		foreach ($bodycopy_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$bodycopies[] = $link_info['value'];
			}
		}

		// ATTENTION!
		// this check relies on the fact that ALL single asset formats like type, default or position
		// are located in subfolders of this asset, and only its context bodycopies are its direct children
		if (empty($bodycopies)) {
			parent::onRequestKeywords($broadcaster, $vars);
			return;
		}

		// work out if forms are customised
		$search_bodycopy_link	= $this->getFormatBodycopyLink('search_form');
		$search_customised		= !empty($search_bodycopy_link) && $search_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;
		$sis_customised			= !empty($sis_bodycopy_link) && $sis_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;

		foreach ($bodycopies as $bodycopy) {
			if ($bodycopy == 'initial' || $bodycopy == 'sis_form' || $bodycopy == 'search_form') {
				// work out the query field keywords
				$fields = $this->attr('fields');
				foreach ($fields as $field => $field_data) {
					$keywords[$field.'_query'] = translate('sch_page_keyword_search_field', $field);
					if ($bodycopy == 'initial') {
						$keywords[$field.'_query_logic'] = translate('sch_page_keyword_search_field_logic', $field);
					}
				}
				// work out search category keywords
				foreach ($this->attr('categories') as $cat_name => $cat_details) {
					$keywords[$cat_name.'_category_chooser'] = 'Dropdown box for category set '.$cat_name;
				}
			}

			// add in the buttons for the customisation bodycopies
			if ($bodycopy == 'search_form') {
				$keywords['submit_button'] = translate('cms_listing_keyword_submit_button');
			}
			if ($bodycopy == 'sis_form') {
				$keywords['search_in_search_button'] = translate('sch_page_keyword_sis_button');
			}

			if ($bodycopy != 'sis_form' && $bodycopy != 'search_form') {
				foreach ($this->keywords[$bodycopy] as $keyword) {
					$keywords[$keyword] = ucwords(str_replace('_', ' ', $keyword));
				}

				// customised forms
				if ($search_customised) {
					$keywords += $this->_getCustomisedFormKeywords();
				}

				// search in search
				if ($sis_customised && $bodycopy == 'results') {
					$keywords += $this->_getSearchInSearchKeywords();
				}
			}
		}//end foreach

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Get additional keywords to be used in the single-asset format
	*
	* Provides keywords to be used alongside those provided by assets themselves.
	* These keywords are understood only by this asset
	*
	* @return array
	* @access protected
	*/
	function _getAdditionalSingleAssetFormatKeywords()
	{
		$keywords = parent::_getAdditionalSingleAssetFormatKeywords();

		$keywords['result_number'] = translate('sch_page_keyword_result_number');
		$keywords['score'] = translate('sch_page_keyword_search_score');

		return $keywords;

	}//end _getAdditionalSingleAssetFormatKeywords()


	/**
	* Get the keyword for Customised Form
	*
	* @return array
	* @access protected
	*/
	function _getCustomisedFormKeywords()
	{
		$keywords = Array();

		$keywords['search_form'] = translate('sch_page_keyword_search_form');

		return $keywords;

	}//end _getCustomisedFormKeywords()


	/**
	* Get the keyword for Search In Search
	*
	* @return array
	* @access protected
	*/
	function _getSearchInSearchKeywords()
	{
		$keywords = Array();

		$keywords['search_form'] = translate('sch_page_keyword_search_form');
		$keywords['search_in_search_form'] = translate('sch_page_keyword_sis_form');
		$keywords['search_in_search_button'] = translate('sch_page_keyword_sis_button');

		return $keywords;

	}//end _getSearchInSearchKeywords()


	/**
	* Return search results based on supplied terms after expanding the specified field into synonyms
	*
	* @param array		$original_search_results	Results as returned from the original unmodified search query
	* @param array		$original_search_info		The original search query "info" structure
	* @param string		$auto_ss_field				The name of the field assigned for "Auto Synonym Search" search. Only terms corresponding to this
	*												field will be looked up for synonym matches and modified accordingly
	* @param Thesaurus	$field_name					The Thesaurus asset with which to perform synonym lookups
	*
	* @return array
	* @access public
	*/
	public function getMultiWordSynonymSearchResults(Array $original_search_results, Array $original_search_info, $auto_ss_field, Thesaurus $thesaurus)
	{
		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

		$search_field = $original_search_info['fields'][$auto_ss_field];
		$words = $search_field['words'];
		if (!is_array($words)) $words = explode(' ', $words);

		// Loop over each group of words (2 words or more) and add synonyms of these multi-word-words to the search string
		$syn_search_info = $original_search_info;

		$synonym_words = Array();
		$word_combinations = $this->_getWordCombinations($words);

		$search_logic = Array();
		$search_string = $syn_search_info['fields'][$auto_ss_field]['words'];

		// If we are just two words, don't forget to check each term as a synonym too (eg; fred flautist -> fred (no match) -> flautist (flute player) = fred flautist flute player)
		if (count($words) == 2) {
			$word_combinations[] = $words[0];
			$word_combinations[] = $words[1];
		}

		foreach ($word_combinations as $word_combination) {
			$combination_synonyms = $thesaurus->getSynonymsForTerm($word_combination);
			$num_synonyms = count($combination_synonyms);
			$synonym_words += $combination_synonyms;

			// Keep track of search logic
			if ($num_synonyms > 0) {
				$search_logic[$word_combination] = $combination_synonyms;

				// Remove word combination from search string
				$search_string = str_ireplace($word_combination, '', $search_string);
				}
		}

		// Only modify the search query if we have matching synonyms
		if (!empty($synonym_words)) {
			// Append found synonyms to the current search
			// For example:
			//		original search = fred flute player
			//		found synonym   = flautist (for 'flute player')
			//		new search      = fred flute.player flautist

			$syn_search_results = Array();

			// We need to ensure that the matching multi-word terms are separated with AND logic
			// First, perform a search for any search terms that are not synonyms or have not been matched with synonyms
			$syn_search_info['fields'][$auto_ss_field]['words'] = $search_string;

			// Keep note of the outer logic, as we have to ensure that we eliminate some results that are not common if it is AND
			$outer_word_logic = $syn_search_info['fields'][$auto_ss_field]['word_logic'];

			// Perform a search for terms which did not match synonyms (eg; "fred" in "fred flute player")
			$initial_search_results = Array();
			if (!empty($search_string)) {
				$initial_search_results = $sm->processSearch($syn_search_info);
			}

			// If we are using AND word logic and can't match the outer terms (eg; "fred") then unfortunately any
			// synonym expansion is not going to return any results common to all terms. Time to exit here with our
			// original search result as we haven't found anything else of use
			if (($outer_word_logic == 'AND') && (empty($initial_search_results))) {
				return $original_search_results;
			}

			// Keep an overall tally
			$common_search_results = Array();
			foreach ($initial_search_results as $result => $weighting) {
				$common_search_results[][$result] = $weighting;
			}

			// Now perform a search for each synonym match so we can aggregate these at the end with AND
			// eg; (flute.player + flautist)
			foreach ($search_logic as $original_term => $matching_synonyms) {
				// Search for user-supplied term (flute player)
				$syn_search_info = $original_search_info;
				$syn_search_info['fields'][$auto_ss_field]['words'] = $original_term;
				$term_exploded = explode(' ', $original_term);

				// Multiple words, use AND logic
				if (count($term_exploded) > 1) {
					$syn_search_info['fields'][$auto_ss_field]['word_logic'] = 'AND';
				}

				$term_search_results = Array();

				$results = $sm->processSearch($syn_search_info);
				if (count($results) > 0) {
					foreach ($results as $result => $weighting) {
						if (isset($term_search_results[$result])) {
							$term_search_results[$result] += $weighting;
						} else {
							$term_search_results[$result] = $weighting;
						}
					}
				}

				// Search for synonym terms (flautist)
				$synonym_term_search_results = Array();
				foreach ($matching_synonyms as $matching_synonym) {
						$syn_search_info = $original_search_info;
						$syn_search_info['fields'][$auto_ss_field]['words'] = $matching_synonym;
						$term_exploded = explode(' ', $matching_synonym);

						// Multiple words, use AND logic
						if (count($term_exploded) > 1) {
							$syn_search_info['fields'][$auto_ss_field]['word_logic'] = 'AND';
						}

						$results = $sm->processSearch($syn_search_info);
						if (count($results) > 0) {
							foreach ($results as $result => $weighting) {
								if (isset($term_search_results[$result])) {
									$term_search_results[$result] += $weighting;
								} else {
									$term_search_results[$result] = $weighting;
								}
							}
						}
				}

				$common_search_results[] = $term_search_results;
			}

			// Flatten common search results. These are the results from our synonym term searches
			$synonym_search_results = Array();
			if ($outer_word_logic == 'AND') {
				// We need to eliminate any items that are not common between results, adding weightings of "keepers" as we go
				$common = Array();
				$first_time = TRUE;
				foreach ($common_search_results as $result) {
					if ($first_time) {
						if (!empty($result)) {
							$common = array_keys($result);
						}
					}

					foreach ($result as $asset_id => $weighting) {
						// Only add to results the second time around if we have common terms
						// This was added for increased complexity
						if ($first_time || (!$first_time && in_array($asset_id, $common))) {
							if (isset($synonym_search_results[$asset_id])) {
								$synonym_search_results[$asset_id] += $weighting;
							} else {
								$synonym_search_results[$asset_id] = $weighting;
							}
						}
					}

					$first_time = FALSE;
				}
			} else {
				// Simply flatten and add
				foreach ($common_search_results as $result) {
					foreach ($result as $asset_id => $weighting) {
						if (isset($synonym_search_results[$asset_id])) {
							$synonym_search_results[$asset_id] += $weighting;
						} else {
							$synonym_search_results[$asset_id] = $weighting;
						}
					}
				}
			}

			// Combine all term results with those of the original search
			foreach ($original_search_results as $result => $weighting) {
				if (isset($synonym_search_results[$result])) {
					$synonym_search_results[$result] += $weighting;
				} else {
					$synonym_search_results[$result] = $weighting;
				}
			}

			// Tell the wider world!
			$original_search_results = $synonym_search_results;
		}

		return $original_search_results;

	}//end getMultiWordSynonymSearchResults


	/**
	* This func will sanitize (remove '%') the keywords if they
	* are in user input and the options is selected to sanitize 'em
	*
	* @param array	$queries	array of queries that are supposed to be processedA
	*
	* @return array
	* @access private
	*/
	private function _sanitizeQueries(Array $queries)
	{
		foreach($queries as $field => $query) {
			if(is_array($query)) {
				$queries[$field] = $this->_sanitizeQueries($query);
			} else {
				// sanitise it by removing '%' from keywords so that
				// they dont go ahead and try to fetch it replacement
				$query = str_replace(chr(0), '' , $query);
				$keywords = extract_keywords($query);
				foreach ($keywords as $keyword) {
					$pattern = '%'.$keyword.'%';
					$query = str_replace($pattern, $keyword, $query);
				}
				$queries[$field] = $query;
			}
		}

		return $queries;

	}//end _sanitizeQueries()


	/**
	* This function will generate an array of the search var to process
	* depending on the GET/POST variable defined and the search terms passed
	*
	*
	* @return array
	* @access public
	*/
	function processSearchVars()
	{
		$search_vars = Array();
		// If the search is stored, then process the search and show the results
		// The initial check is here, so it will still allow sis searches
		$pmap = $this->getAttribute('stored_query_session');
		$pmap_params = $pmap->getParameters();
		$fields = $this->attr('fields');
		$this->_tmp['jump_to_result'] = FALSE;
		if (is_array($pmap_params) && !empty($pmap_params)) {
			foreach ($pmap_params as $params) {
				$sq_index = $pmap->getParameterValue($params);

				if (!is_null($sq_index)) {
					replace_global_keywords($sq_index);
					// If a pipe separated list remove
					$sq_index = str_replace('|', ' ', $sq_index);

					// Now set these variables into $_REQUEST so search_page can read them directly
					if ($params == 'stored_query') {
						// Direct match to all fields
						foreach ($fields as $fieldname => $fieldvalue) {
							$search_vars[$fieldname] = $sq_index;
						}
					} else {
						// Else match the request to the field
						$search_vars[$params] = $sq_index;
					}//end if

					// This stops the search if set and empty
					if (isset($this->_tmp['searched_queries']) && empty($this->_tmp['searched_queries'])) {
						unset($this->_tmp['searched_queries']);
					}

					// Parameter Value loaded, so go to the Results page if the attribute is set
					$this->_tmp['jump_to_result'] = TRUE;

				}//end if
			}//end foreach
		}//end if

		return $search_vars;

	}//end processSearchVars()


}//end class

?>
