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
* $Id: comment_prefs.inc,v 1.1 2006/12/20 03:04:36 bcaldwell Exp $
*
*/

require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';
require_once SQ_ATTRIBUTES_PATH.'/int/int.inc';

/**
* Comment_Prefs
*
* Purpose
*
*    Looks after the creation the Comment Preferences
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Comment_Prefs extends Prefs
{


	/**
	* Overview of preferences handled by this file
	*
	* @var array
	* @access public
	*/
	var $pref_vars = Array(
						'SQ_COMMENT_INPUT_SIZE'	=> Array(
													'name'			=> 'Comment Input Box Size',
													'description'	=> 'This preference allows you to change the size of the Comment Input Box',
													'default'		=> Array(
																		'width'		=> '300px',
																		'height'	=> '100px',
																	   ),
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_MAX_LENGTH'	=> Array(
													'name'			=> 'Comment Maximum Length',
													'description'	=> 'This preferences allows you to set the maximum number of characters that can be entered in the Comment Input Box',
													'default'		=> 0,
													'protected'		=> FALSE,
												   ),
					 );

	/**
	* Attribute for painting width input method
	*
	* @var object
	* @access private
	*/
	var $css_dimension_width;


	/**
	* Attribute for painting height input method
	*
	* @var object
	* @access private
	*/
	var $css_dimension_height;


	/**
	* Attribute for painting max length input method
	*
	* @var object
	* @access private
	*/
	var $int_max_length;


	/**
	* Constructor
	*
	*/
	function Comment_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

		// we'll use these attributes to print the inputs for SQ_COMMENT_INPUT_SIZE
		$this->css_dimension_width =& new Asset_Attribute_CSS_Dimension();
		$this->css_dimension_height =& new Asset_Attribute_CSS_Dimension();

		// attribute for input of SQ_COMMENT_MAX_LENGTH
		$this->int_max_length =& new Asset_Attribute_Int();
		$this->int_max_length->setParam('allow_negative', FALSE);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
		$prefs = array_keys($this->pref_vars);
		if (!is_null($pref) && in_array($pref, array_keys($this->pref_vars))) {
			$prefs = array($pref);
		}

		foreach ($prefs as $pref) {
			$fn = 'paint'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($pref)))).'Backend';
			if (!method_exists($this, $fn)) continue;

			// call individual paint function for each pref
			$this->$fn($o, $have_lock, $pref);
		}

	}//end paintBackend()


	/**
	* Paints the interface for configuring the size of the comment input box
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return void
	* @access public
	*/
	function paintSqCommentInputSizeBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openField(translate('comment_pref_input_size'));

			$comment_size = $this->pref_vars['SQ_COMMENT_INPUT_SIZE']['default'];

			?><table class="sq-backend-table" style="width: 350px;">
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('width') ?></td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock && $is_admin) {
						$this->css_dimension_width->paint($pref.'_width', $have_lock, $comment_size['width']);
					} else {
						echo $comment_size['width'];
					}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('height') ?></td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock && $is_admin) {
						$this->css_dimension_height->paint($pref.'_height', $have_lock, $comment_size['height']);
					} else {
						echo $comment_size['height'];
					}
					?>
					</td>
				</tr>
			</table><?php

		$o->closeField();

	}//end paintSqCommentInputSizeBackend()


	/**
	* Paints the interface for configuring the maximum comment length
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return void
	* @access public
	*/
	function paintSqCommentMaxLengthBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openField(translate('comment_pref_max_length'));

			$comment_length = $this->pref_vars['SQ_COMMENT_MAX_LENGTH']['default'];

			if ($have_lock && $is_admin) {
				$this->int_max_length->value = $comment_length;
				$this->int_max_length->paint($pref.'_max_length', !$have_lock);
			} else {
				echo $comment_length;
			}
			$o->note(translate('comment_pref_max_length_note'));

		$o->closeField();

	}//end paintSqCommentMaxLengthBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, $have_lock, $pref=NULL)
	{
		$prefs = array_keys($this->pref_vars);
		if (!is_null($pref) && in_array($pref, array_keys($this->pref_vars))) {
			$prefs = array($pref);
		}

		$res = TRUE;

		foreach ($prefs as $pref) {
			$fn = 'process'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($pref)))).'Backend';
			if (!method_exists($this, $fn)) continue;

			// call individual process function for each pref
			if (!$this->$fn($o, $have_lock, $pref)) $res = FALSE;
		}
		return $res;

	}//end processBackend()


	/**
	* Processes the interface for configuring the size of the comment input box
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentInputSizeBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) return FALSE;

		if (isset($_POST[$pref.'_width']) && isset($_POST[$pref.'_height'])) {
			// process css attributes
			$this->css_dimension_width->process($pref.'_width');
			$this->css_dimension_height->process($pref.'_height');

			$this->pref_vars['SQ_COMMENT_INPUT_SIZE']['default']['width'] = $this->css_dimension_width->getContent();
			$this->pref_vars['SQ_COMMENT_INPUT_SIZE']['default']['height'] = $this->css_dimension_height->getContent();

			return TRUE;
		}
		return FALSE;

	}//end processSqCommentInputSizeBackend()


	/**
	* Processes the interface for configuring the maximum comment length
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentMaxLengthBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) return FALSE;

		if (isset($_POST[$pref.'_max_length'])) {

			$this->int_max_length->process($pref.'_max_length');
			$this->pref_vars['SQ_COMMENT_MAX_LENGTH']['default'] = $this->int_max_length->value;

			return TRUE;
		}
		return FALSE;

	}//end paintSqCommentMaxLengthBackend()


}//end class

?>
