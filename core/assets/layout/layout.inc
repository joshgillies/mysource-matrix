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
* $Id: layout.inc,v 1.1.2.3 2004/04/21 10:55:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy.inc';

/**
* Layout
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout extends Bodycopy
{

	/**
	* An array of keywords used in this layout
	* @var array
	*/
	var $layout_keywords = Array();


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Layout($assetid=0)
	{
		$this->Bodycopy($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{

		return Array(SQ_LINK_TYPE_1 => Array('asset' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_2 => Array('asset' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_3 => Array('asset' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array('asset' => Array('card' => 'M', 'exclusive' => false))
					);

	}//end _getAllowedLinks()


	/**
	* Returns an associative array of keywords to be replaced for the layout of an asset
	*
	* An example for of the array might be:
	*
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
	* @return array(string => array)
	* @access public
	*/
	function getLayoutKeywords()
	{
		if (empty($this->layout_keywords)) {
			
			$keywords = $this->getKeywords();
			if (empty($keywords)) return Array();

			// initialize all parts of the array
			$this->layout_keywords['screens']			= Array();
			$this->layout_keywords['entire_screens']	= Array();
			$this->layout_keywords['custom']			= Array();
			$this->layout_keywords['layout']			= Array();

			foreach ($keywords as $keyword) {
				
				if (false !== (strpos($keyword, '-'))) {
					list($screen, $tag) = explode('-', $keyword);
					$screen = strtolower($screen);
					$tag = strtolower($tag);

					if ($screen == '__custom') {
						// this is not acually a screen, but a custom keyword that we need to
						// ask the owner of this layout at limbo time
						$this->layout_keywords['custom'][] = $tag;
					}

					if ($tag) {
						// get the first letter of the type
						$type = $tag{0};
						// check to see if we have a field or a section
						if (strtoupper($type) != 'S' && strtoupper($type) != 'F') continue;
						$tag = substr($tag, 2);
						
						if (!isset($this->layout_keywords['screens'][$screen])) $this->layout_keywords['screens'][$screen] = Array();
						
						// its a field
						if (strtoupper($type) == 'F') {
							if (!isset($this->layout_keywords['screens'][$screen]['fields'])) $this->layout_keywords['screens'][$screen]['fields'] = Array();
							if (!in_array($tag, $this->layout_keywords['screens'][$screen]['fields'])) {
								$this->layout_keywords['screens'][$screen]['fields'][$tag] = $keyword;
							}
						// its a section
						} else {
							if (!isset($this->layout_keywords['screens'][$screen]['sections'])) $this->layout_keywords['screens'][$screen]['sections'] = Array();
							if (!in_array($tag, $this->layout_keywords['screens'][$screen]['sections'])) {
								$this->layout_keywords['screens'][$screen]['sections'][$tag] = $keyword;
							}
						}//end if
					// they want the entire screen
					} else {
						if (!in_array($screen, $this->layout_keywords['entire_screens'])) {
							$this->layout_keywords['entire_screens'][$screen] = $keyword;
						}
					}
					
				} else {
					// this keyword is a custom keyword for the layout
					$this->layout_keywords['layout'][] = $keyword;
				}
				
			}//end foreach

		}//end if

		return $this->layout_keywords;

	}//end getLayoutKeywords()


}//end class
?>