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
* $Id: layout.inc,v 1.13 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy.inc';

/**
* Layout
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout extends Bodycopy
{


	/**
	* An array of keywords used in this layout
	* @var array
	*/
	public $layout_keywords = Array();


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
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns an associative array of keywords to be replaced for the layout of an asset
	*
	* An example for of the array might be:
	* <PRE>
	*   Array(
	*      'screens' => Array (
	*           'details' => Array (
	*              'fields' => Array(
	*                 'name'       => 'original_keyword',
	*                 'short_name' => 'original_keyword',
	*              ),
	*              'sections' => Array(
	*                 'edit_contents'  => 'original_keyword',
	*                 'paint_contents' => 'original_keyword',
	*              ),
	*           ),
	*           'contents' => Array(
	*              'fields' => Array(
	*                 'name'       => 'original_keyword',
	*                 'short_name' => 'original_keyword',
	*              ),
	*           ),
	*      ),
	*      'entire_screens' => Array(
	*          'details'  => 'original_keyword',
	*          'contents' => 'original_keyword',
	*      ),
	*      'custom' => Array(
	*          'contents',
	*      ),
	*      'layout' => Array(
	*          'screen_menu',
	*          'commit_button',
	*      ),
	*   );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	public function getLayoutKeywords()
	{
		if (empty($this->layout_keywords)) {

			$keywords = $this->getKeywords();
			if (empty($keywords)) return Array();

			// initialize all parts of the array
			$this->layout_keywords['screens']			= Array();
			$this->layout_keywords['entire_screens']	= Array();
			$this->layout_keywords['custom']			= Array();
			$this->layout_keywords['layout']			= Array();

			$this->layout_keywords = $this->getLayoutKeywordsArray($keywords, $this->layout_keywords);

		}//end if

		return $this->layout_keywords;

	}//end getLayoutKeywords()


	/**
	* returns associative array for the keyword passed 
	*
	* @param string	$keyword		the keyword to be processed
	* @param array	$return_array	the array to return value in
	*
	* @return array
	* @access public
	*/
	public function getLayoutKeywordsArray($keywords, $return_array = Array())
	{
		foreach ($keywords as $keyword) {
			if (FALSE !== (strpos($keyword, '-'))) {
				list($screen, $tag) = explode('-', $keyword);
				$screen = strtolower($screen);
				$tag = strtolower($tag);

				if ($screen == '__custom') {
					// this is not acually a screen, but a custom keyword that we need to
					// ask the owner of this layout at limbo time
					$return_array['custom'][] = $tag;
				}

				if ($tag) {
					if (strpos($tag, 'id_f_') === 0) {
						// Its layout field's id keyword replacement
						if (!isset($return_array['screens'][$screen]['field_ids'])) {
							$return_array['screens'][$screen]['field_ids'] = Array();
						}
						$tag = substr($tag, 5);
						if (!in_array($tag, $return_array['screens'][$screen]['field_ids'])) {
							$return_array['screens'][$screen]['field_ids'][$tag] = $keyword;
						}
						continue;
					}

					// get the first letter of the type
					$type = $tag{0};
					// check to see if we have a field or a section
					if (strtoupper($type) != 'S' && strtoupper($type) != 'F') {
						continue;
					}
					$tag = substr($tag, 2);

					if (!isset($return_array['screens'][$screen])) {
						$return_array['screens'][$screen] = Array();
					}

					// its a field
					if (strtoupper($type) == 'F') {
						if (!isset($return_array['screens'][$screen]['fields'])) {
							$return_array['screens'][$screen]['fields'] = Array();
						}
						if (!in_array($tag, $return_array['screens'][$screen]['fields'])) {
							$return_array['screens'][$screen]['fields'][$tag] = $keyword;
						}
					// its a section
					} else {
						if (!isset($return_array['screens'][$screen]['sections'])) {
							$return_array['screens'][$screen]['sections'] = Array();
						}
						if (!in_array($tag, $return_array['screens'][$screen]['sections'])) {
							$return_array['screens'][$screen]['sections'][$tag] = $keyword;
						}
					}//end if
				// they want the entire screen
				} else {
					if (!in_array($screen, $return_array['entire_screens'])) {
						$return_array['entire_screens'][$screen] = $keyword;
					}
				}

			} else {
				// this keyword is a custom keyword for the layout
				$return_array['layout'][] = $keyword;
			}
		}//end foreach

		return $return_array;
	
	}//end getLayoutKeywordsArray()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param Asset	$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	public function onRequestKeywords(Asset $broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'layout', TRUE);
		if (!empty($parents) && isset($parents[$this->id])) {
			$vars['keywords']['screen_menu'] = 'Screen Menu';
			$vars['keywords']['context_switcher'] = 'Context Switcher';
			$vars['keywords']['commit_button'] = 'Commit Button';
			$vars['keywords']['commit_button_clean'] = 'Commit Button (without formatting)';
			$vars['keywords']['release_locks_button'] = 'Release-locks button';
			$vars['keywords']['xx-'] = 'All of screen xx';
			$vars['keywords']['xx-S_yy'] = 'All of section yy of screen xx';
			$vars['keywords']['xx-F_zz'] = 'Field zz of screen xx';
			$vars['keywords']['__custom-pp'] = 'Custom limbo keyword pp of the current asset type';
		}

	}//end onRequestKeywords()


}//end class
?>
