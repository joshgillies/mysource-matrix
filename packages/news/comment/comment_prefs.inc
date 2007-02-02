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
* $Id: comment_prefs.inc,v 1.5 2007/02/02 05:21:37 arailean Exp $
*
*/

require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';
require_once SQ_ATTRIBUTES_PATH.'/int/int.inc';
require_once SQ_ATTRIBUTES_PATH.'/boolean/boolean.inc';
require_once SQ_ATTRIBUTES_PATH.'/url/url.inc';


if (!defined('SQ_COMMENT_HTML_ALLOW')) {
	define ('SQ_COMMENT_HTML_ALLOW',	0);
}

if (!defined('SQ_COMMENT_HTML_ESCAPE')) {
	define ('SQ_COMMENT_HTML_ESCAPE',	1);
}

if (!defined('SQ_COMMENT_HTML_STRIP')) {
	define ('SQ_COMMENT_HTML_STRIP',	2);
}

/**
* Comment_Prefs
*
* Purpose
*
*    Looks after the creation the Comment Preferences
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.5 $
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
						'SQ_COMMENT_HTML_CONF'	=> Array(
													'name'			=> 'HTML Treatment in comments',
													'description'	=> 'Determines whether HTML is allowed, escaped or removed',
													'default'		=> 0,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_MAX_RATING'	=> Array(
													'name'			=> 'Maximum Rating',
													'description'	=> 'This preference sets the upper bound for ratings',
													'default'		=> 5,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_IMG_RATED'	=> Array(
													'name'			=> 'Rated Image',
													'description'	=> 'Rated Image to be used instead of the default',
													'default'		=> NULL,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_IMG_BLANK'	=> Array(
													'name'			=> 'Blank Image',
													'description'	=> 'Blank Image to be used instead of the default',
													'default'		=> NULL,
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
	* Attribute for painting max length input method
	*
	* @var object
	* @access private
	*/
	var $int_max_rating;


	/**
	* Helper attribute for processing URLs collection interfaces
	*
	* @var object
	* @access private
	*/
	var $url_processor;


	var $html_conf_options = Array();


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

		// attribute for input of SQ_COMMENT_MAX_RATING
		$this->int_max_rating =& new Asset_Attribute_Int();
		$this->int_max_rating->setParam('allow_negative', FALSE);
		$this->int_max_rating->setParam('allow_empty', FALSE);

		$this->url_processor =& new Asset_Attribute_Url();
		$this->url_processor->_edit_params['protocols'] = Array('http','https');
		$this->url_processor->_edit_params['width'] = '60';

		$this->html_conf_options = Array(
									SQ_COMMENT_HTML_ALLOW	=> translate('allow'),
									SQ_COMMENT_HTML_ESCAPE	=> translate('escape'),
									SQ_COMMENT_HTML_STRIP	=> translate('strip'),
								   );

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
			$prefs = Array($pref);
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
	* Paints the interface for allowing HTML in comments
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return void
	* @access public
	*/
	function paintSqCommentHtmlConfBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openField(translate('comment_pref_html_conf_label'));

			$current_conf = $this->pref_vars[$pref]['default'];

			$readonly = !($have_lock && $is_admin);

			if ($readonly) {
				echo array_get_index($this->html_conf_options, $current_conf);
			} else {
				combo_box($pref, $this->html_conf_options, FALSE, $current_conf);
			}

		$o->closeField();

	}//end paintSqCommentHtmlConfBackend()


	/**
	* Paints the interface for allowing HTML in comments
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return void
	* @access public
	*/
	function paintSqCommentMaxRatingBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openField($this->pref_vars[$pref]['name']);

		$max_rating = $this->pref_vars[$pref]['default'];

		$readonly = !($have_lock && $is_admin);

		$this->int_max_rating->value = $max_rating;
		$this->int_max_rating->paint($pref, $readonly);

		$o->closeField();

	}//end paintSqCommentMaxRatingBackend()


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
			$prefs = Array($pref);
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
		if (!$is_admin || !$have_lock) {
			return FALSE;
		}

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
		if (!$is_admin || !$have_lock) {
			return FALSE;
		}

		if (isset($_POST[$pref.'_max_length'])) {

			$this->int_max_length->process($pref.'_max_length');
			$this->pref_vars['SQ_COMMENT_MAX_LENGTH']['default'] = $this->int_max_length->value;

			return TRUE;
		}
		return FALSE;

	}//end processSqCommentMaxLengthBackend()


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
	function processSqCommentHtmlConfBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) {
			return FALSE;
		}

		$new_setting = array_get_index($_POST, $pref);

		if (!is_null($new_setting)) {
			$this->pref_vars[$pref]['default'] = $new_setting;

			return TRUE;
		}
		return FALSE;

	}//end processSqCommentHtmlConfBackend()


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
	function processSqCommentMaxRatingBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) {
			return FALSE;
		}

		$this->int_max_rating->process($pref);
		if (!$this->int_max_rating->processed) {
			return FALSE;
		}

		$this->pref_vars[$pref]['default'] = $this->int_max_rating->value;

		return TRUE;

	}//end processSqCommentMaxRatingBackend()


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
	function paintSqCommentImgRatedBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openField($this->pref_vars[$pref]['name']);

		$current_setting = $this->pref_vars[$pref]['default'];

		$read_only = !($have_lock && $is_admin);

		$image_location_prefix = sq_web_path('data').'/asset_types/comment/files';

		if (empty($current_setting)) {
			$current_image = sq_web_path('data').'/asset_types/comment/files/star.png';
		} else {
			$current_image = $current_setting;
		}

		echo 'Current Image: <img src="'.htmlspecialchars($current_image).'" />';
		echo '<br />';

		$url = $this->url_processor;
		$url->value = $current_setting;
		$url->paint($pref, $read_only);


		if (!$read_only) {
			$o->note($this->pref_vars[$pref]['description']);
			$o->note('Empty means default');
		}

		$o->closeField();

	}//end paintSqCommentImgRatedBackend()


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
	function processSqCommentImgRatedBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) {
			return FALSE;
		}

		$url = $this->url_processor;

		$url->process($pref);
		$new_setting = $url->value;

		$this->pref_vars[$pref]['default'] = $new_setting;

		return TRUE;

	}//end processSqCommentMaxRatingBackend()


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
	function paintSqCommentImgBlankBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openField($this->pref_vars[$pref]['name']);

		$current_setting = $this->pref_vars[$pref]['default'];

		$read_only = !($have_lock && $is_admin);

		$image_location_prefix = sq_web_path('data').'/asset_types/comment/files';

		if (empty($current_setting)) {
			$current_image = sq_web_path('data').'/asset_types/comment/files/star_grey.png';
		} else {
			$current_image = $current_setting;
		}

		echo 'Current Image: <img src="'.htmlspecialchars($current_image).'" />';
		echo '<br />';

		$url = $this->url_processor;
		$url->value = $current_setting;
		$url->paint($pref, $read_only);

		if (!$read_only) {
			$o->note($this->pref_vars[$pref]['description']);
			$o->note('Empty means default');
		}

		$o->closeField();

	}//end paintSqCommentImgBlankBackend()


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
	function processSqCommentImgBlankBackend(&$o, $have_lock, $pref)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) {
			return FALSE;
		}


		$url = $this->url_processor;

		$url->process($pref);
		$new_setting = $url->value;

		$this->pref_vars[$pref]['default'] = $new_setting;

		return TRUE;

	}//end processSqCommentImgBlankBackend()


}//end class

?>
