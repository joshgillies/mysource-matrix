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
* $Id: search_page.inc,v 1.52 2004/09/02 01:13:24 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc'; // for the medatata date field selection

/**
* Search_Page
*
* Page template for searching
*
* @see Page
*
* @author  Dominic Wong <dwong@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.52 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page extends Page
{

	/**
	* An array of bodycopies and their names that are attached underneath this page asset
	* @var array
	*/
	var $bodycopies = Array (
							'initial'			=> 'Initial Search Page Layout',
							'results'			=> 'Results Page Layout',
							'result_element'	=> 'Result Element Layout',
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
		'initial' => Array(
			'submit_button'		=> Array('description' => '<input type="button" value="Search" />', 'type' => 'special'),
			'results_per_page'	=> Array('description' => '<input type="text" value="Results per page" />', 'type' => 'special'),
								),
		'results' => Array(
			'result_list'				=> Array('description' => 'List of search results', 'type' => 'block'),
			'result_count'				=> Array('description' => 'Number of results found', 'type' => 'inline'),
			'current_result_page'		=> Array('description' => 'Page of results we are on', 'type' => 'inline'),
			'total_result_pages'		=> Array('description' => 'Total number of result pages', 'type' => 'inline'),
			'result_page_index'			=> Array('description' => 'List of result pages', 'type' => 'inline'),
			'previous_result_page_link'	=> Array('description' => 'Previous', 'type' => 'inline'),
			'next_result_page_link'		=> Array('description' => 'Next', 'type' => 'inline'),
			'initial_layout'			=> Array('description' => 'New search', 'type' => 'block'),
			'category_list_linked'		=> Array('description' => 'Linked list of categories', 'type' => 'inline'),
								),
		'result_element' => Array(
			'score'						=> Array('description' => 'Result score', 'type' => 'inline'),
			'result_number'				=> Array('description' => 'No.', 'type' => 'inline'),
			'asset_id'					=> Array('description' => 'Asset ID', 'type' => 'inline'),
			'asset_name'				=> Array('description' => 'Name of the asset', 'type' => 'inline'),
			'asset_name_linked'			=> Array('description' => 'Name of the asset (linked)', 'type' => 'inline'),
			'asset_short_name'			=> Array('description' => 'Short name of the asset', 'type' => 'inline'),
			'asset_short_name_linked'	=> Array('description' => 'Short name of the asset (linked)', 'type' => 'inline'),
			'asset_url'					=> Array('description' => 'URL of the asset', 'type' => 'inline'),
			'asset_lineage'				=> Array('description' => 'Lineage to the asset', 'type' => 'inline'),
			'asset_lineage_linked'		=> Array('description' => 'Lineage to the asset (linked)', 'type' => 'inline'),
								),
		);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Page($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
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

		// add a bodycopy for the initial search page layout, result page layout, and result rows
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		foreach ($this->bodycopies as $value => $name) {
			$copy_link = Array(
				'asset' => &$this,
				'link_type' => SQ_LINK_TYPE_2,
				'dependant' => 1,
				'exclusive' => 1,
				'value'		=> $value
			);

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			if (!$bodycopy->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string			$side_of_link	the side of the link we are on (major or minor)
	* @param string			$link_type		the type of link we are creating
	* @param string			$value			the value that is to be associated with the link
	* @param string			$sort_order		the position in the links list that this link should take,
	* @param string			$dependant		'0' / '1' on whether the this asset is dependant on the
	*										asset that will be linked by the new link
	* @param string			$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*										to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if a bodycopy is linking to us then we need to make it a dependant link
		if ($side_of_link == 'major' && is_a($asset, 'bodycopy') && $dependant != '1') {
			$dependant = '1';
			return true;
		}

		return false;

	}//end prepareLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		
		if ($bodycopy_link['minor_type_code'] == 'bodycopy' && !$GLOBALS['SQ_PURGING_TRASH']) return 'You can\'t delete the link between the search page and its bodycopies';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => count($this->bodycopies), 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_2]['folder'] = Array('card' => 1, 'exclusive' => false);
		$page_links[SQ_LINK_TYPE_3]['folder'] = Array('card' => 1, 'exclusive' => false);
		$page_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => false);
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
				return 'The root asset from where searches are conducted from';
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()
	
	
	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()
	
	
	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		$mode = array_get_index($_REQUEST, 'mode', 'initial');
		if (!isset($_SESSION['SQ_LAST_SEARCH'][$this->id])) {
			$_SESSION['SQ_LAST_SEARCH'][$this->id] = Array();
		} else {
			if ($mode != 'sis') $_SESSION['SQ_LAST_SEARCH'][$this->id] = Array();
		}
		
		// get the corresponding bodycopy attached to us
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		$current_result_page = array_get_index($_REQUEST, 'current_result_page', 0);
		$results_per_page    = array_get_index($_REQUEST, 'results_per_page', $this->attr('results_per_page'));

		?>
		<form name="search_form" method="get" action="<?php echo $this->getUrl(); ?>">
			<input type="hidden" name="mode" value="<?php echo $mode; ?>" />
			<input type="hidden" name="current_result_page" value="<?php echo $current_result_page; ?>" />
			<input type="hidden" name="results_per_page" value="<?php echo $results_per_page; ?>" />
			<?php
			switch ($mode) {
				case 'results':
				case 'sis'    :
					$this->printResultsBody();
				break;

				case 'initial':
				default:
					echo $this->getBodycopyContents('initial', $this->getInitialBodyReplacements());
				break;
			}
			?>
		</form>
		<?php

	}//end printBody()


	/**
	* Prints the results bodycopy
	*
	* @access private
	*
	* @return void
	*/
	function printResultsBody()
	{
		$results = $this->processSearch();
		$replacements = array_merge($this->getInitialBodyReplacements(), $this->getResultsBodyReplacements($results));
		echo $this->getBodycopyContents('results', $replacements);

	}//end printResultsBody()


	/**
	* Retrieves a bodycopy by name
	*
	* @param string		$name	the name of the bodycopy to retrieve
	*
	* @return [object|null] The bodycopy, or null if the name does not match any bodycopies.
	* @access private
	* @see Search_Page::bodycopies
	*/
	function &getBodycopy($name)
	{
		if (array_key_exists($name, $this->bodycopies)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			$link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, $name, 'major', '1');
			$bodycopy =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			return $bodycopy;
		} else {
			return null;
		}

	}//end getBodycopy()


	/**
	* Retrieves a bodycopy's contents by name
	*
	* @param string		$name	the name of the bodycopy to retrieve
	*
	* @return [string|null] the bodycopy's contents, or empty string if the name does not match any bodycopies
	* @access private
	* @see Search_Page::bodycopies
	* @see Search_Page::getBodycopy
	*/
	function &getBodycopyContents($name, $replacements=Array())
	{
		$bodycopy =& $this->getBodycopy($name);
		if (is_null($bodycopy)) return '';

		$bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getBodycopyContents()


	/**
	* Getting the date-time field
	*
	* @param	string				$prefix	fieldname for form elements
	* @param	object Asset		&$asset	the owning asset
	* @param	string				$value	field value, can be null
	*
	* @return object DateTime_Field
	* @access private
	*/
	function getEpochDateTimeField($fieldname, &$asset, $value=null)
	{
		// only if we are printing the default value
		if (is_null($value)) {
			if (!is_null($asset)) {
				$value = (int) $asset->attr('default');
			} else {
				$value = '%asset_created%'; // just testing
			}
		}

		$epoch_date = getdate($value);
		$value_arr = Array(
						'y' => $epoch_date['year'],
						'm' => $epoch_date['mon'],
						'd' => $epoch_date['mday'],
						'h' => $epoch_date['hours'],
						'i' => $epoch_date['minutes'],
						's' => $epoch_date['seconds'],
					);

		// parameters
		if (!is_null($asset)) {
			$parameters = $asset->attr('setup_params');
		}
		if (empty($parameters) || is_null($parameters)) {
			$date = getdate();
			$parameters = Array(
				'min'			=> '2003-01-01 00:00:00',
				'max'			=> ($date['year'] + 1).'-12-31 23:59:59',
				'allow_circa'	=> '0',
				'print_format'	=> $asset->attr('format'),
				'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
				'null'			=> Array(), // allowing nulls everywhere
				'style'			=> Array(
								'y' => 's',
								'm' => 's',
								'd' => 's',
								'h' => 's',
								'i' => 's',
								's' => 's',
							),
			);
		}

		$value = '';
		$field = new DateTime_Field($fieldname, $value, $parameters);
		$value = $field->unitsArrayToStr($value_arr);
		$field->setValue($value);

		return $field;

	}// end _getEpochDateTimeField()


	/**
	* Returns the replacement for the specified field
	*
	* @param string $field name the name of the field to get the replacement for
	*
	* @return string
	* @access public
	*/
	function getFieldReplacement($field_name)
	{
		
		$query_var = $field_name.'_query';
		$fields = $this->attr('fields');
		$query = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $query_var, '');

		ob_start();

			if ($fields[$field_name]['type'] == 'metadata') {

				$assetid = $fields[$field_name]['params']['metadata_field']['assetid'];
				$am = &$GLOBALS['SQ_SYSTEM']->am;
				$asset = &$am->getAsset($assetid);
				$params = $asset->vars;

				if (isset($params['format'])) { // metadata Date field
					// don't ask me why I'm doing this
					$temp_name = $query_var.'__';
					$field = $this->getEpochDateTimeField("$temp_name", $asset);
					$field->processField(); // showing value if any
					$field->printField();
				} else if (isset($params['multi_line'])) { // normal metadata TEXT field
					// using same defintion as for a simple field, but will have to revise later...
					text_box("queries_$query_var", $query, $this->defaults['query_width']);
				} else if (isset($params['select_options'])) { // metadata dropdown selection
					$select_options = $params['select_options']['value'];
					$select_options = array_reverse($select_options);
					$select_options[''] = '- Please select -';
					$select_options = array_reverse($select_options);
					combo_box('queries_'.$query_var, $select_options, false, $query);
				}

			} else if ($fields[$field_name]['type'] == 'asset_attrib') {

				$search_styles = $this->attr('search_styles');

				// [0] - name, [1] - type, [2] - id
				$attr_parts = explode('__', $fields[$field_name]['params']['type_attr']);

				// if this is a boolean field, we want to make sure
				// the default value is '_' instead of a blank string
				// so the correct option is selected for '-- Leave Empty --'
				if ($attr_parts[1] == 'boolean') if ($query == '') $query = '_';

				require_once SQ_ATTRIBUTES_PATH.'/'.$attr_parts[1].'/'.$attr_parts[1].'.inc';
				$attr_class = 'Asset_Attribute_'.$attr_parts[1]; // attr name
				$attribute = new $attr_class($attr_parts[2], $query); // attr id

				if (!empty($search_styles) && isset($search_styles[$attribute->name])) {
					$attribute->_edit_params = $search_styles[$attribute->name];
				}

				if ($attr_parts[1] == 'selection') {
					if (empty($attribute->_params['multiple'])) {
						$attribute->_params['options'] = array_reverse($attribute->_params['options']);
						$attribute->_params['options']['_'] = '-- Leave Empty --';
						$attribute->_params['options'] = array_reverse($attribute->_params['options']);
					}
				}

				$attribute->paint('queries_'.$query_var, false, false);

			} else {

				// not a metadata field, just a normal text field
				text_box('queries_'.$query_var, $query, $this->defaults['query_width']);

			}

			$replacement = ob_get_contents();

		ob_end_clean();

		return $replacement;

	}//end getFieldReplacement()


	/**
	* Returns the replacement for the specified field in a search in search form
	*
	* @param string $field name the name of the field to get the replacement for
	*
	* @return string
	* @access public
	*/
	function getSisFieldReplacement($field_name)
	{
		$query_var = $field_name.'_query';
		$query = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $query_var, '');
		$query_var = $field_name.'_query_sis';

		$fields = $this->attr('fields');

		ob_start();

			if ($fields[$field_name]['type'] == 'metadata') {

				$assetid = $fields[$field_name]['params']['metadata_field']['assetid'];
				$am = &$GLOBALS['SQ_SYSTEM']->am;
				$asset = &$am->getAsset($assetid);
				$params = $asset->vars;

				if (isset($params['format'])) {
					// metadata date field
					$temp_name = $query_var.'__';
					$field = $this->getEpochDateTimeField("$temp_name", $asset);
					
					// showing value if any
					$field->processField();
					$field->printField();
				} else if (isset($params['multi_line'])) {
					// normal metadata text field
					text_box('queries_'.$query_var, '', $this->defaults['query_width']);
				} else if (isset($params['select_options'])) {
					// metadata dropdown selection
					$select_options = $params['select_options']['value'];
					if (empty($query) || !isset($select_options[$query])) {
						$select_options = array_reverse($select_options);
						$select_options[''] = '- Please select -';
						$select_options = array_reverse($select_options);
						combo_box('queries_'.$query_var, $select_options, false, $query);
					} else {
						echo $select_options[$query];
					}
				}

			} else if ($fields[$field_name]['type'] == 'asset_attrib') {

				$search_styles = $this->attr('search_styles');
				$read_only = false;

				// [0] - name, [1] - type, [2] - id
				$attr_parts = explode('__', $fields[$field_name]['params']['type_attr']);

				// if this is a boolean field, we want to make sure
				// the default value is '_' instead of a blank string
				// so the correct option is selected for '-- Leave Empty --'
				if ($attr_parts[1] == 'boolean') if ($query == '') $query = '_';

				require_once SQ_ATTRIBUTES_PATH.'/'.$attr_parts[1].'/'.$attr_parts[1].'.inc';
				$attr_class = 'Asset_Attribute_'.$attr_parts[1]; // attr name
				$attribute = new $attr_class($attr_parts[2], $query); // attr id

				if (!empty($search_styles) && isset($search_styles[$attribute->name])) {
					$attribute->_edit_params = $search_styles[$attribute->name];
				}
				
				switch ($attr_parts[1]) {
					case 'selection' :
						if (empty($attribute->_params['multiple'])) {
							if (isset($attribute->_params['options'][$query])) {
								$read_only = true;
							} else {
								$attribute->_params['options'] = array_reverse($attribute->_params['options']);
								$attribute->_params['options']['_'] = '-- Leave Empty --';
								$attribute->_params['options'] = array_reverse($attribute->_params['options']);
							}
						} else {
							if (!empty($query)) {
								echo '<div class="search_current_options">';
								foreach ($query as $option) {
									if (!isset($attribute->_params['options'][$option])) continue;
									echo $attribute->_params['options'][$option].'<br />';
									unset($attribute->_params['options'][$option]);
								}
								echo '</div>';
							}
						}
					break;

					case 'boolean' :
						if ($query != '_') $read_only = true;
					break;

					case 'text' :
						$attribute->value = '';
					break;
				}//end switch
				
				$attribute->paint('queries_'.$query_var, $read_only, false);

			} else {

				// not a metadata field, just a normal text field
				text_box('queries_'.$query_var, '', $this->defaults['query_width']);

			}

			$replacement = ob_get_contents();

		ob_end_clean();

		return $replacement;

	}//end getSisFieldReplacement()


	/**
	* Returns replacements that that are common to the bodycopies
	*
	* @param string $keyword the keyword of the wanted replacement
	*
	* @return string
	* @access public
	*/
	function getGeneralReplacement($keyword)
	{
		$replacement = '';
		switch ($keyword) {

			case 'search_button' :
			case 'submit_button' :
				$replacement  = '<input type="submit" onclick="
					document.search_form.current_result_page.value=\'0\';
					document.search_form.mode.value = \'results\';
					return true;
				" value="'. $this->attr('submit_button_text').'" />';
			break;

			case 'search_in_search_button' :
				$replacement  = '<input type="submit" onclick="
					document.search_form.current_result_page.value=\'0\';
					document.search_form.mode.value = \'sis\';
					return true;
				" value="'. $this->attr('sis_button_text').'" />';
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

			case 'category_list' :
				$category = (isset($_REQUEST['search_category'])) ? $_REQUEST['search_category'] : '';
				$categories = $this->attr('categories');
				if (!empty($categories)) {
					$options = Array('' => $this->attr('no_category_text'));
					foreach ($categories as $key => $data) $options[$key] = $data['name'];
					ob_start();
						combo_box('search_category', $options, false, $category);
						$replacement = ob_get_contents();
					ob_end_clean();
				} else {
					$replacement = '';
				}
			break;

			case 'sort_by' :
				$sort_by = (isset($_REQUEST['sort_by'])) ? $_REQUEST['sort_by'] : $this->attr('sort_by');
				$sort_attr = $this->getAttribute('sort_by');
				ob_start();
					combo_box('sort_by', $sort_attr->_params['options'], false, $sort_by);
					$replacement = ob_get_contents();
				ob_end_clean();
			break;

			case 'reverse_sort' :
				$reverse = (isset($_REQUEST['reverse_sort'])) ? $_REQUEST['reverse_sort'] : $this->attr('reverse_sort');
				ob_start();
					combo_box('reverse_sort', Array('1' => 'Yes', '0' => 'No'), false, $reverse);
					$replacement = ob_get_contents();
				ob_end_clean();
			break;

			case 'results_per_page' :
				$results_per_page = $_REQUEST['results_per_page'];
				$replacement = '<input type="text" onchange="
					document.search_form.results_per_page.value = this.value;
				" value="'.$results_per_page.'" />';
			break;

			default :
				$replacement = "%$keyword%";
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
		$keywords = $bodycopy->getKeywords();
		$replaces = Array();

		$fields = $this->attr('fields');

		foreach ($keywords as $keyword) {

			$replacement =& $replaces[$keyword];
			if (ereg('(.+)_query', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				$replacement = $this->getFieldReplacement($matches[1]);
			} else {
				$replacement = $this->getGeneralReplacement($keyword);
			}

		}//end foreach keywords

		return $replaces;

	}//end getInitialBodyReplacements()


	/**
	* Returns the keyword replacements for the specified format bodycopy
	*
	* @param string $form_name the name of the form to get the replacements for
	*
	* @return Array()
	* @access public
	*/
	function getFormFormatReplacements($form_name)
	{
		$bodycopy = &$this->getFormatBodycopy($form_name);
		if (is_null($bodycopy)) return Array();

		$fields = $this->attr('fields');
		$keywords = $bodycopy->getKeywords();

		$replacements = Array();
		foreach ($keywords as $keyword) {
			if (ereg('(.+)_query', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				if ($form_name == 'sis_form') {
					$replacements[$keyword] = $this->getSisFieldReplacement($matches[1]);
				} else {
					$replacements[$keyword] = $this->getFieldReplacement($matches[1]);
				}
			} else {
				
				if ($form_name == 'sis_form') {
					switch ($keyword) {
						case 'category_list':
							$category = (isset($_REQUEST['search_category'])) ? $_REQUEST['search_category'] : '';
							$categories = $this->attr('categories');
							$replacements[$keyword] = ($category) ? $categories[$category]['name'] : $this->attr('no_category_text');
						break;

						default :
							$replacements[$keyword] = $this->getGeneralReplacement($keyword);
						break;
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
	* @param string 	$form_name 		the name of the format to get the contents for
	* @param Array		$replacements 	the array of replacements to replace in the content
	*
	* @return string
	* @access public
	*/
	function getFormFormatContents($form_name, $replacements)
	{
		$bodycopy = &$this->getFormatBodycopy($form_name);
		if (is_null($bodycopy)) return '';
		
		$bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getFormFormatContents()


	/**
	* Keyword-replaces for the results mode.
	*
	* @param array	$results	the results returned
	*
	* @return void
	* @access private
	*/
	function getResultsBodyReplacements(&$results)
	{
		$bodycopy =& $this->getBodycopy('results');
		$keywords = $bodycopy->getKeywords();
		$replaces = Array();

		$current_result_page = $_REQUEST['current_result_page'];
		$results_per_page    = $_REQUEST['results_per_page'];

		$total_pages = ceil(count($results) / $results_per_page);

		$queries = '';

		$queries_array = Array();
		foreach($_REQUEST as $key => $value) {
			if (substr($key, 0, 8) == 'queries_') {
				$suffix = substr($key, 8);
				$queries_array[$suffix] = $_REQUEST[$key];
			}
		}

		foreach ($queries_array as $q_name => $q_value) {
			$queries .= '&queries_'.htmlentities_array($q_name).'='.htmlentities_array($q_value);
		}

		foreach ($keywords as $keyword) {
			$replacement =& $replaces[$keyword];
			$replacement = "%$keyword%";

			switch ($keyword) {
				case 'result_list':
					$replacement = '';
					$sub_resultids = array_slice(array_keys($results), $current_result_page * $results_per_page, $results_per_page);
					$result_num = $current_result_page * $results_per_page;
					
					$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
					foreach ($sub_resultids as $assetid) {
						$result_num++;
						
						$element_contents = $cm->loadFromCache($this, $assetid);
						if (empty($element_contents)) {
							$element_replacements = $this->getResultElementBodyReplacements($assetid);
							// make sure the bodycopy doesnt replace the dynamic keywords with blank strings
							// or else the cached version will never have these keywords replaced properly
							// we willbe replacing them ourselves later on
							$element_replacements['score'] = '%score%';
							$element_replacements['result_number'] = '%result_number%';
							$element_contents = $this->getBodycopyContents('result_element', $element_replacements);
							$cm->saveToCache($this, $assetid, $element_contents);
						}

						$element_replacements = Array('score'         => $results[$assetid],
													  'result_number' => $result_num
													  );
						$replacement .= replace_keywords($element_contents, $element_replacements);
					}
				break;

				case 'result_count':
					$replacement = count($results);
				break;

				case 'current_result_page':
					$replacement = min($current_result_page + 1, $total_pages);
				break;

				case 'total_result_pages':
					$replacement = $total_pages;
				break;

				case 'result_page_index':
					$replacement = '';
					for ($i = 0; $i < $total_pages; $i++) {
						if ($replacement != '')
							$replacement .= '&nbsp;';

						if ($i == $current_result_page) {
							$replacement .= "<b>".($i + 1)."</b>";
						} else {
							$replacement .= '<a href="'.$this->getURL().'?mode=results&current_result_page='.$i.'&results_per_page='.$results_per_page.$queries.'">'.($i + 1)."</a>";
						}
					}
				break;

				case 'previous_result_page_link':
					if ($current_result_page < 1) {
						$replacement = 'Previous';
						break;
					}
					$replacement = '<a href="'.$this->getURL().'?mode=results&current_result_page='.($current_result_page - 1).'&results_per_page='.$results_per_page.$queries.'">Previous</a>';
				break;

				case 'next_result_page_link':
					if ($current_result_page >= $total_pages - 1) {
						$replacement = 'Next';
						break;
					}
					$replacement = '<a href="'.$this->getURL().'?mode=results&current_result_page='.($current_result_page + 1).'&results_per_page='.$results_per_page.$queries.'">Next</a>';
				break;

				case 'initial_layout':
					$replacement = $this->getBodycopyContents('initial', $this->getInitialBodyReplacements());
				break;
				
				default:
					if (substr($keyword, -7) == '_linked') {
						if ($keyword == 'category_list_linked') {
							// printing the list of seach categories
							$categories = $this->attr('categories');
							if (!empty($categories)) {
								$current_category = array_get_index($_REQUEST, 'search_category', '');
								if (!empty($current_category) && !isset($categories[$current_category])) $current_category = '';
								
								$base_url = $_SERVER['PHP_SELF'].'?'.$_SERVER['QUERY_STRING'];
								$base_url = preg_replace('|([&?])mode=([^&]+)|', '\\1mode=results', $base_url);
								$base_url = preg_replace('|&search_category=([^&]+)?|', '', $base_url);
								$base_url .= '&search_category=';
								ob_start();
									?>
									<div id="search_category_list">
										<ul>
											<li<?php echo (empty($current_category)) ? ' id="current"' : ''; ?>>
												<a href="<?php echo $base_url; ?>"><?php echo $this->attr('no_category_text'); ?></a>
											</li>
										<?php
										foreach ($categories as $key => $data) {
											?>
											<li<?php echo ($current_category == $key) ? ' id="current"' : ''; ?>>
												<a href="<?php echo $base_url.$key; ?>"><?php echo $data['name']; ?></a>
											</li>
											<?php
										}
										?>
										</ul>
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
			
								$assetid = $fields[$query_var]['params']['metadata_field']['assetid'];
								$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								$params = $asset->vars;
								// metadata dropdown selection
								if (isset($params['select_options'])) $options = $params['select_options']['value'];
			
							} else if ($fields[$query_var]['type'] == 'asset_attrib') {
			
								$search_styles = $this->attr('search_styles');

								// [0] - name, [1] - type, [2] - id
								$attr_parts = explode('__', $fields[$query_var]['params']['type_attr']);
								
								require_once SQ_ATTRIBUTES_PATH.'/'.$attr_parts[1].'/'.$attr_parts[1].'.inc';
								$attr_class = 'Asset_Attribute_'.$attr_parts[1];
								$attribute = new $attr_class($attr_parts[2], $query_var);
								
								if (isset($search_styles[$attribute->name]['search_not_selected_text'])) {
									$not_selected_text = $search_styles[$attribute->name]['search_not_selected_text'];
								}
								
								if ($attr_parts[1] == 'selection') {
									if (empty($attribute->_params['multiple'])) $options = $attribute->_params['options'];
								} else if ($attr_parts[1] == 'boolean') {
									if (isset($search_styles[$attribute->name])) {
										$options = Array('0' => $search_styles[$attribute->name]['false_text'],
														 '1' => $search_styles[$attribute->name]['true_text']
														 );
									} else {
										$options = Array('0' => 'False', '1' => 'True');
									}
								}
							}
							
							if (!empty($options)) {
								$query_key = 'queries_'.$query_var.'_query';
								$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $query_var.'_query', '_');
								
								$base_url = $_SERVER['PHP_SELF'].'?'.$_SERVER['QUERY_STRING'];
								$base_url = preg_replace('|([&?])mode=([^&]+)|', '\\1mode=results', $base_url);
								$base_url = preg_replace('|&'.$query_key.'=([^&]+)?|', '', $base_url);
								$base_url .= '&'.$query_key.'=';
								
								if ($not_selected_text == '') $not_selected_text = 'No Selection';
								ob_start();
									?>
									<div id="search_<?php echo $query_var; ?>_query_list">
										<ul>
											<li<?php echo ($current_value == '_') ? ' id="current"' : ''; ?>>
												<a href="<?php echo $base_url; ?>_"><?php echo $not_selected_text; ?></a>
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
							}
						}//end if category links

					} else {
						$replacement = $this->getGeneralReplacement($keyword);
					}// end if linked
				break;
			}//end switch

		}//end foreach

		return $replaces;

	}//end getResultsBodyReplacements()


	/**
	* Keyword-replaces for the result element
	*
	* @param int		$assetid		the asset ID of the result element
	*
	* @return void
	* @access private
	*/
	function getResultElementBodyReplacements($assetid)
	{
		$bodycopy =& $this->getBodycopy('result_element');
		$keywords = $bodycopy->getKeywords();

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$asset = &$am->getAsset($assetid);

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$replaces = $mm->generateKeywordReplacements($asset, $keywords, false);
		
		$metadata_keywords = Array();
		foreach ($keywords as $key) {
			if (substr($key, 0, 15) == 'asset_metadata_') $metadata_keywords[$key] = substr($key, 15);
		}
		if (!empty($metadata_keywords)) {
			$metadata_values = $mm->getMetadataFieldValues($assetid, array_values($metadata_keywords));
			foreach ($metadata_keywords as $key => $field) {
				if (isset($metadata_values[$field])) $replaces[$key] = $metadata_values[$field];
			}
		}

		$asset_keywords = $asset->getAssetKeywords();
		foreach ($asset_keywords as $key => $data) {
			if (in_array($key, $keywords)) $replaces[$key] = $data['value'];
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
					$first = true;
					$found_root = false;
					$parent_asset_ids = $am->getParents($asset->id);

					// searching for the root asset
					$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
					$root_assetid = empty($root_link) ? 0 : $root_link['minorid'];

					foreach ($parent_asset_ids as $parent_asset_id) {
						if ($parent_asset_id == $root_assetid) $found_root = true;
						if (!$found_root) continue;
						if (!$first) $replacement .= $this->attr('lineage_seperator');
						$first = false;
						$ancestor_asset = &$am->getAsset($parent_asset_id);
						if ($keyword == 'asset_lineage_linked') $replacement .= '<a href="'.$ancestor_asset->getUrl().'">';
						$replacement .= $ancestor_asset->name;
						if ($keyword == 'asset_lineage_linked') $replacement .= '</a>';
						$am->forgetAsset($ancestor_asset);
					}
				break;
			}//end switch
		}//end foreach

		return $replaces;

	}//end getResultElementBodyReplacements()


	/**
	* Processes a search request
	*
	* Note that all search queries are constructed an run in the Search Manager
	*
	* From the posted variables, it formulates search info and returns results
	*
	* @return array
	* @access public
	* @see Search_Manager:processSearch()
	*/
	function processSearch()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$sm = &$am->getSystemAsset('search_manager');

		$fields = $this->attr('fields');

		$mode = array_get_index($_REQUEST, 'mode', 'results');
	
		$suffix = ($mode == 'results') ? '_query' : '_query_sis';

		$queries = Array();
		foreach($_REQUEST as $key => $value) {
			$matches = Array();
			preg_match('|^queries_(.*)'.$suffix.'$|', $key, $matches);
			if (empty($matches)) continue;
			$queries[$matches[1].'_query'] = $value;
		}

		// check if there are medatadata date fields|
		foreach (array_keys($_REQUEST) as $req_key) {
			if (ereg('(.+)__value', $req_key, $query_match)) {
				$newind = $query_match[1];
				$reqind = $query_match[0];
				// if found, adding its values to general queries array
				$queries[$newind] = $_REQUEST[$reqind];
			}
		}

		if ($mode == 'sis') {
			$last_search = $_SESSION['SQ_LAST_SEARCH'][$this->id];

			foreach ($queries as $query_var => $value) {

				if (is_scalar($value)) $value = trim($value);
				$field_name = substr($query_var, 0, -6);

				if ($fields[$field_name]['type'] == 'asset_attrib') {
					// if this is a selection or boolean attribute, the value may actually be
					// an '_' - indicating that it is not searched, but we want to remove this and
					// use our own value
					$attr_parts = explode('__', $fields[$field_name]['params']['type_attr']);
					if ($attr_parts[1] == 'selection' || $attr_parts[1] == 'boolean') {
						if (isset($last_search[$query_var]) && $last_search[$query_var] == '_') {
							$last_search[$query_var] = '';
						}
					}
				}

				if (!isset($last_search[$query_var]) || $last_search[$query_var] == '') {
					$last_search[$query_var] = $value;
				} else {
					// trim the current value before we append more words to it
					if (is_scalar($last_search[$query_var])) {
						$last_search[$query_var] = trim($last_search[$query_var]);
					}
					if (!is_array($value)) {
						$last_search[$query_var] .= ' '.$value;
					} else {
						$last_search[$query_var] = array_merge($value, $last_search[$query_var]);
					}
				}
			}
			$queries = $last_search;
		}

		// save the search terms into the session so that we can use them for
		// search within search results
		$_SESSION['SQ_LAST_SEARCH'][$this->id] = $queries;

		// search category
		$category = (isset($_REQUEST['search_category'])) ? $_REQUEST['search_category'] : '';
		$categories = $this->attr('categories');
		if (!empty($category) && !isset($categories[$category])) $category = '';
		
		// get the basic search settings based on the current category (if any)
		$search_info = $this->populateBaseSearchInfo($category);

		// adding in the search terms for our search fields
		foreach($queries as $query_var => $value) {

			if ($value == '') continue;
			if ($value == '_') continue; // for boolean submissions...

			if (!isset($fields[substr($query_var, 0, -6)])) continue;
			$field = $fields[substr($query_var, 0, -6)];
			
			// checking if the boolean value needs adjustments
			if ($field['type'] == 'asset_attrib') {
				if (strpos($field['params']['type_attr'], 'boolean') !== false) {
					// we need to convert the value into a word representation, 
					// since indexing service will not operate just 0 or 1
					if ($value == 0) $value = 'false';
					else $value = 'true';
				}
			}

			if (!is_array($value)) {
				$words = split(' +', strtolower($value));
				foreach (array_keys($words) as $i) {
					// convert wildcard symbols to corresponding SQL wildcards
					$words[$i] = strtr($words[$i], '*?', '%_');

					// if this is an email, skip the following check...
					$at_pos  = strpos($words[$i], '@');
					$dot_pos = strpos($words[$i], '.');
					if ($at_pos === false && $dot_pos === false) {
						// preserve the wildcard symbols - i.e. don't quote them
						if (ereg('^[A-z%_]+$', $words[$i])) $words[$i] = addslashes($words[$i]);
						else unset($words[$i]);
					} // if this is an email
				}
			} else {
				// if $value is array and the field is a metadata field, then the
				// metadata field is a datetime field and needs to have its value
				// entered into an array - otherwise, the current $value array is fine
				if ($field['type'] == 'metadata') $words = Array($value);
				else $words = $value;
			}

			$search_info[$field['type']][$field['name']]['words'] = $words;
			$search_info[$field['type']][$field['name']]['params'] = (isset($field['params'])) ? $field['params'] : Array();
		}

		return $sm->processSearch($search_info);

	}//end processSearch()


	/**
	* Get the basic search settings based on the current category
	*
	* @param string	$category	the current search category
	*
	* @return boolean
	* @access private
	*/
	function populateBaseSearchInfo($category='')
	{
		$search_info = Array();

		$search_info['logic'] = $this->attr('logic');

		$link_value = 'root';
		if (!empty($category)) $link_value .= '_'.$category;
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, $link_value);
		$search_info['root_assetid'] = empty($root_link) ? 0 : $root_link['minorid'];

		// sort settings can be user defined as well as being set in the backend
		$search_info['sort_by']      = (isset($_REQUEST['sort_by'])) ? $_REQUEST['sort_by'] : $this->attr('sort_by');
		$search_info['reverse_sort'] = (isset($_REQUEST['reverse_sort'])) ? $_REQUEST['reverse_sort'] : $this->attr('reverse_sort');
		
		$search_info['search_types'] = $this->attr('search_types');
		if (!empty($category) && isset($categories[$category]['search_statuses'])) {
			$search_info['search_statuses'] = $categories[$category]['search_statuses'];
		} else {
			$search_info['search_statuses'] = $this->attr('search_status');
		}
		
		return $search_info;

	}//end populateBaseSearchInfo()


	/**
	* Returns the link for the format folder
	*
	* @return Array()
	* @access public
	*/
	function getFormatFolderLink()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'folder', true);
		return (!empty($link)) ? $link[0] : Array();

	}//end getFormatFolderLink()


	/**
	* Returns the format folder where the format bodycopies live
	*
	* @return &object asset
	* @access public
	*/
	function &getFormatFolder()
	{
		$link = $this->getFormatFolderLink();
		if (empty($link)) {
			$null = null;
			return $null;
		}

		$folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $folder;

	}//end getFormatFolder()


	/**
	* Returns the link for a bodycopy with the specified link value
	*
	* @param string $link_value the link value of the link of the wanted bodycopy
	*
	* @return Array()
	* @access public
	*/
	function getFormatBodycopyLink($link_value)
	{
		$format_folder = &$this->getFormatFolder();
		if (is_null($format_folder)) return Array();

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($format_folder->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', true, 'major', $link_value);
		return (!empty($link)) ? $link[0] : Array();

	}//end getFormatBodycopyLink()


	/**
	* Returns the format bodycopy with the specifed link value
	*
	* If the bodycopy does not exist, or it have a link type 3, null is returned
	* as either no format has been customised, or the format feature has been disbled
	*
	* @param string $link_value the link value of the wanted bodycopy
	*
	* @return &object asset
	* @access public
	*/
	function &getFormatBodycopy($link_value)
	{
		$bodycopy_link = $this->getFormatBodycopyLink($link_value);
		if (empty($bodycopy_link)) {
			$null = null;
			return $null;
		}

		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		return $bodycopy;
	
	}//end getFormatBodycopy()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;
		if (!is_a($broadcaster, 'content_type_wysiwyg')) return;

		// lets work out which bodycopy the WYSIWYG cell that wants our keywords
		// is in so we can return the correct keywords for the screen
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);

		$bodycopies = Array();
		foreach ($bodycopy_links as $link_info) {
			if (in_array($link_info['minorid'], $parents)) {
				$bodycopies[] = $link_info['value'];
			}
		}
		if (empty($bodycopies)) return;

		$keywords = Array();
		foreach ($bodycopies as $bodycopy) {
			if ($bodycopy == 'initial') {
				// work out the query replacements
				$fields = $this->attr('fields');
				foreach ($fields as $field => $field_data) {
					$name = 'Search Field ('.$field.')';
					$description = '<input type="text" width="'.$this->defaults['query_width'].'" value="Search terms here" />';
					$keywords[$field.'_query'] = Array('name' => $name, 'default' => $description, 'type' => 'special');
				}
			} else if ($bodycopy == 'result_element') {
				$default_keywords = $broadcaster->getAssetKeywords(true);
				foreach ($default_keywords as $keyword => $info) {
					$name = ucwords(str_replace('_', ' ', $keyword));
					$keywords[$keyword] = Array('name' => $name, 'default' => $info['description'], 'type' => 'inline');
				}
			}
			foreach ($this->keywords[$bodycopy] as $keyword => $info) {
				$name = ucwords(str_replace('_', ' ', $keyword));
				$keywords[$keyword] = Array('name' => $name, 'default' => $info['description'], 'type' => $info['type']);
			}
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


}//end class

?>
