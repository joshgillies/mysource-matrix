<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: paint_layout_bodycopy.inc,v 1.38 2008/08/22 01:19:43 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy.inc';
require_once 'include/conditions_list.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Paint_Layout_Bodycopy
*
* A Text/HTML Template for Assets
* Represents an independant and customizable presentation layers for a type of
* asset.
*
* The content of the bodycopy can hold 2 types of keywords:
*
*	Substitution keywords:
*		. %asset_attribute_XXX%
*		. %asset_metadata_XXX%
*		. %asset_content%
*		. other custom keywords
*
*	Conditional keywords:
*		. %begin_KEYWORD%
*		. %else_KEYWORD%
*		. %end_KEYWORD%
*		(where KEYWORD is a substitution keyword)
*
* If an attribute is not recognised, the condition will be evaluated as FALSE.
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.38 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Bodycopy extends Bodycopy
{

	var $asset_to_paint = NULL;

	var $conditions_list = NULL;

	var $keyword_replacements = Array();


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
	* Saves the paint layout bodycopy's reusable data to its data directory
	*
	* @return boolean
	* @access public
	*/
	public function _updated()
	{
		// Use bodycopy's save functionality to store any notice links our containers have,
		// and to store the original content before any substitutions take place
		if (!parent::_updated()) return FALSE;

		$this->_generateContentFile();

		return TRUE;

	}//end _updated()


	/**
	* Generate the content file for this paint layout bodycopy
	*
	* @return void
	* @access private
	*/
	protected function _generateContentFile()
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$php_begin = '<'.'?'.'php ';
		$php_end = ' ?'.'>';
		$content_file = '';
		$required_substitution_keywords = Array();

		$containers = $this->getContainers();
		foreach (array_keys($containers) as $i) {
			ob_start();
				$container =& $containers[$i]; // need this by reference
				$edit = $container->getEditFns();
				$edit->paint($container, FALSE, TRUE);
				$content = ob_get_contents();
			ob_end_clean();

			$required_keywords = retrieve_keywords_replacements($content);
			$keyword_replacements = Array();
			$additional_keywords = Array();

			// find all the required keywords and replace the condition-block keywords by PHP code
			foreach ($required_keywords as $keyword) {
				if (!isset($keyword_replacements[$keyword])) {
					$value = NULL;

					if (preg_match('/^('.SQ_KEYWORD_CONDITION_BEGIN.'|'.SQ_KEYWORD_CONDITION_END.'|'.SQ_KEYWORD_CONDITION_ELSE.')_(.*)$/', $keyword, $condition_parts)) {
						if (count($condition_parts) == 3) {
							$test_keyword = $condition_parts[2];
							switch ($condition_parts[1]) {
								case SQ_KEYWORD_CONDITION_BEGIN :
									$this->_addRequiredKeywordsFromEvaluatedKeyword($test_keyword, $required_substitution_keywords);
									$value = 'if (!empty($paint_layout_keywords[\''.$test_keyword.'\']) && trim($paint_layout_keywords[\''.$test_keyword.'\']) != \'\') {';
								break;
								case SQ_KEYWORD_CONDITION_ELSE :
									$value = '} else {';
								break;
								case SQ_KEYWORD_CONDITION_END :
									$value = '} // end '.$test_keyword;
								break;
							}

							if (!empty($value)) {
								$value = $php_begin.$value.$php_end;
							}
						}
					}

					if (isset($value)) {
						$keyword_replacements[$keyword] = $value;
					} else {
						$required_substitution_keywords[$keyword] = 1;
					}
				}
			}//end foreach

			// Replace all the conditional keywords in our content with valid PHP code
			replace_keywords($content, $keyword_replacements);

			// Change each ./?a= url to reference the sq_asset_url_list array, which
			// is built from our container's notice links when content is printed.
			$e = '|\./\?a=([0-9]+)([^0-9])|';
			$with = '<'.'?php echo $sq_asset_url_list[\\1]; ?'.'>\\2';
			$content = preg_replace($e, $with, $content);

			$content_file .= '
							$container = $GLOBALS["SQ_SYSTEM"]->am->getAsset('.$container->id.',  "'.$container->type().'");
							if ($container->readAccess()) {
								'.$php_end.$content.$php_begin.'
							}//end if read access for container '.$container->id.'
							';

		}//end foreach container

		$content_file = $php_begin.$content_file.$php_end;

		// Flatten line endings to a common one, as the sections of content file
		// generated by Matrix and that by the database may have different
		$content_file = str_replace("\r\n","\n", $content_file);
		$content_file = str_replace("\r","\n", $content_file);

		// ...but that ending should really be \r\n; when sent to a browser,
		// text/html MIME-type requires \r\n line endings
		$content_file = str_replace("\n","\r\n", $content_file);

		// save the contents of the bodycopy
		if (!is_dir($this->data_path)) {
			create_directory($this->data_path);
		}
		if (!string_to_file($content_file, $this->data_path.'/content_file.php')) {
			trigger_localised_error('CORE0047', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// Save the substitution keywords we will need to lookup for each asset display
		create_directory($this->data_path);
		if (!string_to_file(serialize($required_substitution_keywords), $this->data_path.'/keywords')) {
			trigger_localised_error('CORE0048', E_USER_WARNING, $this->name);
			return FALSE;
		}

	}//end _generateContentFile()


	/**
	* Returns the body of the painted asset
	*
	* @param Asset	asset	the asset to paint
	*
	* @return string
	* @access public
	*/
	public function paintAsset(Asset $asset)
	{
		/*
		 * Possible keywords:
		 *
		 * SUBSTITUTION_KEYWORDS:
		 *
		 * 1. any asset keywords
		 * 2. %asset_content%
		 *
		 * CONDITIONAL KEYWORDS:
		 *
		 * 3. %begin_SUBSTITUTION_KEYWORD% ... %end_SUBSTITUTION_KEYWORD%
		 *
		 */

		$ret = '';

		$this->asset_to_paint = $asset;

		// Retrieve any assets referenced in this bodycopy, and get their urls for lookup
		if (file_exists($this->data_path.'/.notice_links')) {
			$linked_assets = unserialize(file_to_string($this->data_path.'/.notice_links'));
		} else {
			// nothing cached out from the Bodycopy asset level (ie. not in safe edit) so
			// need to grab our notice links ourselves
			$linked_assets = Array();
			$containers = $this->getContainers();

			foreach ($containers as $container) {
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($container->id, SQ_LINK_NOTICE);
				foreach ($links as $link) {
					$linked_assets[] = $link['minorid'];
				}
			}
			$linked_assets = array_unique($linked_assets);
		}
		$sq_asset_url_list = $GLOBALS['SQ_SYSTEM']->am->getAssetHref($linked_assets);

		// Find stored keyword replacements
		$paint_layout_keywords = Array();
		if (file_exists($this->data_path.'/keywords')) {
			$required_keywords = array_keys(unserialize(file_to_string($this->data_path.'/keywords')));
			$paint_layout_keywords = $this->_getAssetKeywordReplacements($asset, $required_keywords);
		}

		// Read in content file, find keyword replacements, spit out the results
		$content = '';
		if (file_exists($this->data_path.'/content_file.php')) {
			ob_start();
				include($this->data_path.'/content_file.php');
				$content = ob_get_contents();
				replace_keywords($content, $paint_layout_keywords);
			ob_end_clean();
		} else {
			$this->setKeywordReplacements($this->_getAssetKeywordReplacements($asset, $this->getKeywords()));
			ob_start();
				$this->printBody();
				$content = ob_get_contents();
			ob_end_clean();
		}
		return $content;

	}//end paintAsset()


	/**
	* Add to an array all the keywords which value is needed to evaluate a
	* particular keyword
	*
	* @param string	$evaluated_keyword	the keyword that will be evaluated
	* @param array	&$required_keywords	the Array to which the needed keywords will be added
	*
	* @return void
	* @access private
	*/
	protected function _addRequiredKeywordsFromEvaluatedKeyword($evaluated_keyword, Array &$required_keywords)
	{
		// if the evaluated keyword is a logical condition, then gets the logical keywords it is relying on
		$condition_list = new Conditions_List($this, 'conditional_keywords');
		foreach ($condition_list->getRequiredKeywords($evaluated_keyword) as $required_keyword) {
			$required_keywords[$required_keyword] = 1;
		}

		// add at least the keyword that will be evaluated
		$required_keywords[$evaluated_keyword] = 1;

	}//end _addRequiredKeywordsFromEvaluatedKeyword()


	/**
	* Returns TRUE if the value of the keyword is not blank.
	* 0 will be evaluated as TRUE.
	*
	* @param mixed	$keyword_name	a keyword name
	*
	* @return boolean
	* @access public
	*/
	protected function _evaluateKeyword($keyword_name)
	{
		if (!isset($this->keyword_replacements[$keyword_name])) {
			return FALSE;
		}

		$value = trim($this->keyword_replacements[$keyword_name]);

		return ($value != '');

	}//end _evaluateKeyword()


	/**
	* Returns the value of a single keyword
	*
	* @param string	$keyword	keyword name that must be replaced
	*
	* @return string
	* @access private
	*/
	protected function _getKeywordValue($keyword)
	{
		// TODO: must optimise getAssetKeywordReplacements() with a cache
		$ret = $this->_getAssetKeywordReplacements($this->asset_to_paint, Array($keyword));

		$ret = isset($ret[$keyword]) ? $ret[$keyword] : '';

		return $ret;

	}//end _getKeywordValue()


	/**
	* Returns the values of a list of keywords.
	* The returned array may contain more keywords than needed.
	*
	* @param Asset	$asset				the asset which will provide the values for the keywords
	* @param array	$required_keywords	keyword names that must be replaced
	*
	* @return array
	* @access private
	*/
	protected function &_getAssetKeywordReplacements(Asset &$asset, Array $required_keywords)
	{
		$ret = Array();

		// 1. asset content
		if (in_array('asset_contents', $required_keywords)) {
			ob_start();
				$asset->printBody();
				$ret['asset_contents'] = ob_get_contents();
			ob_end_clean();
		}

		// 2. asset keywords (includes attributes and metadata)
		foreach ($required_keywords as $required_keyword) {
			if ($required_keyword != 'asset_contents') {
				$ret[$required_keyword] = $asset->getKeywordReplacement($required_keyword);
			}
		}

		// asset lineage
		if (in_array('asset_lineage', $required_keywords) || in_array('asset_lineage_linked', $required_keywords)) {
			$asset_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL();

			$ret['asset_lineage'] = '';
			$ret['asset_lineage_linked'] = '';
			$lineage_separator = ' > ';
			$lineage_separator = $this->attr('lineage_separator');

			foreach ($asset_lineage as $asset_lineage_item) {
				if (!empty($ret['asset_lineage'])) {
					$ret['asset_lineage'] .= $lineage_separator;
					$ret['asset_lineage_linked'] .= $lineage_separator;
				}
				$ret['asset_lineage'] .= $asset_lineage_item['short_name'];
				$ret['asset_lineage_linked'].='<a href="'.$asset_lineage_item['protocol'].'://'.$asset_lineage_item['url'].'">'.$asset_lineage_item['short_name'].'</a>';
			}
		}

		// 3. Conditions
		// TODO:
		// This will make this harder once the $required_keywords will be really applied by getAssetKeywords()
		// function, because the logical conditions may need other keywords which are not in $required_keywords
		$condition_list = new Conditions_List($this, 'conditional_keywords');
		$ret = array_merge($ret, $condition_list->evaluate($asset, $required_keywords, $ret, Array(FALSE, TRUE)));
		return $ret;

	}//end _getAssetKeywordReplacements()


	/**
	* Sets the associated type code
	*
	* @param string	$asset_type	asset type
	*
	* @return void
	* @access public
	*/
	public function setAssociatedAssetType($asset_type='')
	{
		$this->setAttrValue('associated_asset_type', $asset_type);

	}//end setAssociatedAssetType()


	/**
	* Gets the associated type code
	*
	* @return string
	* @access public
	*/
	public function getAssociatedAssetType()
	{
		return $this->attr('associated_asset_type');

	}//end getAssociatedAssetType()


	/**
	* Fill an array with the list of all available keywords
	*
	* @param array	&$keywords			an array to fill
	* @param string	$asset_type_code	the type of asset which should be represented by this bodycopy.
	*
	* @return void
	* @access public
	*/
	public function getKeywordsDescription(Array &$keywords, $asset_type_code='')
	{
		if (is_array($keywords)) {

			// 1. asset content
			$keywords['asset_contents'] = translate('asset_contents');

			// 2. asset keywords (includes attributes)
			if (!empty($asset_type_code)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type_code);
				$asset = new $asset_type_code();
			} else {
				$asset = new Asset();
			}

			$asset_keywords = $asset->getAvailableKeywords();
			foreach ($asset_keywords as $keyword_name => $description) {
				$keywords[$keyword_name] = (empty($description)) ? ucwords(str_replace('_', ' ', $keyword_name)) : $description;
			}

			unset($asset);

			// lineage
			$keywords['asset_lineage'] = translate('asset_lineage');
			$keywords['asset_lineage_linked'] = translate('asset_lineage_linked');

			// 3. asset metadata
			$keywords['asset_metadata_'] = translate('asset_metadata_x');

			// 4. conditions
			$condition_list = new Conditions_List($this, 'conditional_keywords');
			$condition_names = $condition_list->getConditionsNames();

			foreach ($condition_names as $condition_name) {
				$keywords[$condition_name] = ucwords(str_replace('_', ' ', $condition_name));
				$keywords['begin_'.$condition_name] = translate('core_paint_layout_bodycopy_begin').' '.ucwords(str_replace('_', ' ', $condition_name));
				$keywords['else_'.$condition_name] = translate('core_paint_layout_bodycopy_else').' '.ucwords(str_replace('_', ' ', $condition_name));
				$keywords['end_'.$condition_name] = translate('core_paint_layout_bodycopy_end').' '.ucwords(str_replace('_', ' ', $condition_name));
				$keywords['begin_'.$condition_name.'%<br />%else_'.$condition_name.'%<br />%end_'.$condition_name] = translate('core_paint_layout_bodycopy_full').' '.ucwords(str_replace('_', ' ', $condition_name));
			}

			ksort($keywords);

		}//end if

	}//end getKeywordsDescription()


}//end class

?>
