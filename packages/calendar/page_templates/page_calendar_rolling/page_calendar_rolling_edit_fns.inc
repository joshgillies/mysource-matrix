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
* $Id: page_calendar_rolling_edit_fns.inc,v 1.4 2006/02/15 01:06:48 rhoward Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* Page_Calendar_Rolling_Edit_Fns
*
* Purpose
*	Functions to help print and process the backend interface of rolling calendar page assets
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Rolling_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Calendar_Rolling_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEventSources(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			multiple_asset_finder($prefix.'_event_sources', array_keys($asset->attr('root_nodes')));
			return TRUE;
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($asset->attr('root_nodes')), '', TRUE, 'name');
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $id => $name) {
				echo '<li>'.get_asset_tag_line($id).'</li>';
			}
			?></ul><?php
			return FALSE;
		}

	}//end paintEventSources()


	/**
	* Process the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processEventSources(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_event_sources'])) {
			$root_nodes = Array();
			foreach ($_POST[$prefix.'_event_sources'] as $source) {
				if (!empty($source['assetid'])) {
					$root_nodes[$source['assetid']] = 1;
				}
			}
			return $asset->setAttrValue('root_nodes', $root_nodes);
		}
		return FALSE;

	}//end processEventSources()


//--        Days From/To        --//


	/**
	* Paints the interface for the 'From' attribute
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDaysFrom(&$asset, &$o, $prefix)
	{
		return $this->_paintDaySelect($asset, $prefix, 'days_from');

	}//end paintDaysFrom()


	/**
	* Processes the interface for the 'From' attribute
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDaysFrom(&$asset, &$o, $prefix)
	{
		return $this->_processDaySelect($asset, $prefix, 'days_from');

	}//end processDaysFrom()


	/**
	* Paints the interface for the 'To' attribute
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDaysTo(&$asset, &$o, $prefix)
	{
		return $this->_paintDaySelect($asset, $prefix, 'days_to');

	}//end paintDaysTo()


	/**
	* Processes the interface for the 'To' attribute
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDaysTo(&$asset, &$o, $prefix)
	{
		return $this->_processDaySelect($asset, $prefix, 'days_to');

	}//end processDaysTo()


	/**
	* Paints the controls used by paintDaysTo and paintDaysFrom
	*
	* @param string	&$asset	the asset to which the attribute belongs
	* @param string	$prefix	prefix for the form elements
	* @param string	$attr	the attribute to paint
	*
	* @return boolean
	* @access public
	*/
	function _paintDaySelect(&$asset, $prefix, $attr)
	{
		$write_access = $asset->writeAccess('attributes');

		$pf_select_contents = Array(
								'days_past'		=> translate('cal_rolling_days_past_combo'),
								'days_future'	=> translate('cal_rolling_days_future_combo'),
							  );

		// take the single attribute, break it out into a (positive) textbox value,
		// and a dropdown switching between days in the past, and days in the future
		$attr_val  = $asset->attr($attr);
		$text_val  = abs($attr_val);
		$combo_val = ($attr_val < 0 ? 'days_past' : 'days_future');

		if ($write_access) {
			int_text_box($prefix.'_'.$attr, $text_val, FALSE, 2);
			combo_box($prefix.'_pastfuture_select_'.$attr, $pf_select_contents, FALSE, $combo_val);
		} else {
			echo translate('cal_rolling_'.$combo_val, $text_val);
		}

		return $write_access;

	}//end _paintDaySelect()


	/**
	* Processes the day selection for processDaysTo and processDaysFrom
	*
	* @param string	&$asset	the asset to which the attribute belongs
	* @param string	$prefix	prefix for the form elements
	* @param string	$attr	the attribute to process
	*
	* @return boolean
	* @access public
	*/
	function _processDaySelect(&$asset, $prefix, $attr)
	{
		if ($asset->writeAccess('attributes')) {
			$text_val   = abs(array_get_index($_REQUEST, $prefix.'_'.$attr, 0));
			$select_val = array_get_index($_REQUEST, $prefix.'_pastfuture_select_'.$attr, 'days_future');

			$attr_val = ($select_val == 'days_past' ? ($text_val * -1) : $text_val);

			return $asset->setAttrValue($attr, $attr_val);
		}

		return TRUE;

	}//end _processDaySelect()


}//end class
?>