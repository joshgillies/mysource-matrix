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
* $Id: template_builder_edit_fns.inc,v 1.1 2004/11/02 00:04:54 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Template_Builder_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Template_Builder_Edit_Fns extends Page_Edit_Fns
{
	

	/**
	* Constructor
	*
	*/
	function Template_Builder_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end Template_Builder_Edit_Fns()

	
	/**
	* Paint the 'Question Fields' field
	*
	* @param object Template_Builder	&$asset
	* @param object Backend_Outputter	&$o
	* @param string						$prefix
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionFields(&$asset, &$o, $prefix)
	{
		$fields = $asset->getQuestionFields();
		?><table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					Question Field Name
				</td>
				<td class="sq-backend-table-header">
					Type
				</td>
				<td class="sq-backend-table-header">
					Possible Values
				</td>
				<?php

				foreach($fields as $qf) {
					?>
						<tr>
							<td class="sq-backend-table-cell">
								<?php echo $qf['name'] ?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
								switch ($qf['type']) {
									case '[':
										echo 'Select';
									break;
									
									case '{':
										echo 'Multiple Select';
									break;
									
									case '(':
									case '':
										echo 'Freeform Text';
									break;
									
									default:
										echo '* Unknown *';
									break;
									
								}//end switch
							?></td>
							<td class="sq-backend-table-cell">
								<?php 
								if ($qf['type'] == '(') {
									echo $qf['values'][0].' column(s) x '.$qf['values'][1].' row(s)';
								} else if ($qf['type'] == '') {
									// default for freeform text
									echo 'Default: 40 column(s) x 1 row(s)';
								} else {
									echo implode(', ', $qf['values']);
								} ?>
							</td>
						</tr>
					<?php
				}//end foreach
				?>
				</table><?php
				
		return true;
		
	}//end paintQuestionFields()

	
	/**
	* Process Question Fields field - no processing done here
	*
	* @return boolean	true always
	*/
	function processQuestionFields(&$asset, &$o, $prefix)
	{
		return true;
	}

	
	/**
	* Redirect to the template bodycopy
	*
	* @param object Page_Custom_Form	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToTemplateBodycopy(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$bc_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'major', 'template');
		$bc = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bc_link[0]['minorid']);
		if (is_null($bc)) {
			trigger_error('Could not locate the Template Bodycopy for template builder page "'.$asset->name.'"', E_USER_ERROR);
			return false;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($bc->getBackendHref('contents'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return false;

	}//end redirectToFormContents()


}//end class
?>
