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
* $Id: design_area_js_calendar_navigator_edit_fns.inc,v 1.6 2008/11/24 23:17:14 mbrydon Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_JS_Calendar_Navigator_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Design_Area_JS_Calendar_Navigator_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Paints the interface for choosing target calendar pages
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTargetPages(&$asset, &$o, $prefix)
	{
		$components = Array('year', 'month', 'week', 'day');
		?>
		<table class="sq-backend-table">
			<?php
			foreach ($components as $comp) {
				?>
				<tr>
					<th style="width: 20%"><?php echo ucfirst($comp); ?></th>
					<td><?php $this->paintNoticeLinkedAsset($asset, $comp.'_page', Array('page_calendar' => 'D'), $prefix.$comp); ?></td>
				</tr>
				<?php
			}
			?>
		</table>
		<?php

	}//end paintTargetPages()


	/**
	* Processes the interface for choosing target calendar pages
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTargetPages(&$asset, &$o, $prefix)
	{
		$components = Array('year', 'month', 'week', 'day');
		foreach ($components as $comp) {
			$this->processNoticeLinkedAsset($asset, $comp.'_page', $prefix.$comp);
		}

	}//end processTargetPages()


}//end class
?>