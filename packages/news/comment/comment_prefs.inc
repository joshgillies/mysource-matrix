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
* $Id: comment_prefs.inc,v 1.11 2012/08/30 00:59:01 ewang Exp $
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
* @version $Revision: 1.11 $
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
													'name'			=> 'comment_pref_input_size',
													'description'	=> 'This preference allows you to change the size of the Comment Input Box.',
													'default'		=> Array(
																		'width'		=> '300px',
																		'height'	=> '100px',
																	   ),
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_MAX_LENGTH'	=> Array(
													'name'			=> 'comment_pref_max_length',
													'description'	=> 'This preferences allows you to set the maximum number of characters that can be entered in the Comment Input Box.',
													'default'		=> 0,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_HTML_CONF'	=> Array(
													'name'			=> 'comment_pref_html_conf_label',
													'description'	=> 'Determines whether HTML is allowed, escaped or removed.',
													'default'		=> SQ_COMMENT_HTML_ESCAPE,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_MAX_RATING'	=> Array(
													'name'			=> 'maximum_rating',
													'description'	=> 'This preference sets the upper bound for ratings.',
													'default'		=> 5,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_IMG_RATED'	=> Array(
													'name'			=> 'rated_image',
													'description'	=> 'Rated image to be used instead of the default.',
													'default'		=> NULL,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_IMG_BLANK'	=> Array(
													'name'			=> 'blank_image',
													'description'	=> 'Blank image to be used instead of the default.',
													'default'		=> NULL,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS'	=> Array(
													'name'			=> 'comment_disable_keyword_replacements',
													'description'	=> 'Whether to replace the keywords in the comment.',
													'default'		=> 1,
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
		$this->css_dimension_width = new Asset_Attribute_CSS_Dimension();
		$this->css_dimension_height = new Asset_Attribute_CSS_Dimension();

		// attribute for input of SQ_COMMENT_MAX_LENGTH
		$this->int_max_length = new Asset_Attribute_Int();
		$this->int_max_length->setParam('allow_negative', FALSE);

		// attribute for input of SQ_COMMENT_MAX_RATING
		$this->int_max_rating = new Asset_Attribute_Int();
		$this->int_max_rating->setParam('allow_negative', FALSE);
		$this->int_max_rating->setParam('allow_empty', FALSE);

		$this->url_processor = new Asset_Attribute_Url();
		$this->url_processor->_edit_params['protocols'] = Array('http','https');
		$this->url_processor->_edit_params['width'] = '60';

		$this->disable_keyword_replacements = Array(
												0 => "No",
												1 => "Yes",
											);

		$this->html_conf_options = Array(
									SQ_COMMENT_HTML_ALLOW	=> translate('Allow'),

									SQ_COMMENT_HTML_ESCAPE	=> translate('Escape'),

									SQ_COMMENT_HTML_STRIP	=> translate('Strip'),

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
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		$print_sub_title = TRUE;
		if (!is_null($pref)) $print_sub_title = FALSE;

		$prefs = array_keys($this->pref_vars);
		if (!is_null($pref) && in_array($pref, array_keys($this->pref_vars))) {
			$prefs = Array($pref);
		}

		foreach ($prefs as $pref_value) {
			$fn = 'paint'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($pref_value)))).'Backend';
			if (!method_exists($this, $fn)) continue;

			// call individual paint function for each pref
			if (is_null($pref) || $pref == $pref_value) {
				$this->$fn($o, $have_lock, $pref_value, $print_sub_title, $is_root, $is_admin);
			}
		}

	}//end paintBackend()


	/**
	* Paints the interface for configuring the size of the comment input box
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref_name			a particular preference to paint
	* @param string		$print_sub_title	if TRUE print the sub title
	* @param boolean	$is_root			are we the root user?
	* @param boolean	$is_admin			are we an admin user?
	*
	* @return void
	* @access public
	*/
	function paintSqCommentInputSizeBackend(&$o, $have_lock, $pref_name, $print_sub_title, $is_root, $is_admin)
	{
		if ($print_sub_title) {
			$o->openField(translate($this->pref_vars['SQ_COMMENT_INPUT_SIZE']['name']));
		} else {
			$o->openField('&nbsp;');
		}

			$comment_size = $this->pref_vars['SQ_COMMENT_INPUT_SIZE']['default'];

			?><table class="sq-backend-table" style="width: 350px;">
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Width') ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock && $is_admin) {
						$this->css_dimension_width->paint($pref_name.'_width', $have_lock, $comment_size['width']);
					} else {
						echo $comment_size['width'];
					}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Height') ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock && $is_admin) {
						$this->css_dimension_height->paint($pref_name.'_height', $have_lock, $comment_size['height']);
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
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref_name			a particular preference to paint
	* @param string		$print_sub_title	if TRUE print the sub title
	* @param boolean	$is_root			are we the root user?
	* @param boolean	$is_admin			are we an admin user?
	*
	* @return void
	* @access public
	*/
	function paintSqCommentMaxLengthBackend(&$o, $have_lock, $pref_name, $print_sub_title, $is_root, $is_admin)
	{
		if ($print_sub_title) {
			$o->openField(translate($this->pref_vars['SQ_COMMENT_MAX_LENGTH']['name']));
		} else {
			$o->openField('&nbsp;');
		}

			$comment_length = $this->pref_vars['SQ_COMMENT_MAX_LENGTH']['default'];

			if ($have_lock && $is_admin) {
				$this->int_max_length->value = $comment_length;
				$this->int_max_length->paint($pref_name.'_max_length', !$have_lock);
			} else {
				echo $comment_length;
			}
			$o->note(translate('This setting limits the number of characters that may be entered in the comment input box. When the value is zero, this option will be ignored.'));


		$o->closeField();

	}//end paintSqCommentMaxLengthBackend()


	/**
	* Paints the interface for allowing HTML in comments
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref_name			a particular preference to paint
	* @param string		$print_sub_title	if TRUE print the sub title
	* @param boolean	$is_root			are we the root user?
	* @param boolean	$is_admin			are we an admin user?
	*
	* @return void
	* @access public
	*/
	function paintSqCommentHtmlConfBackend(&$o, $have_lock, $pref_name, $print_sub_title, $is_root, $is_admin)
	{
		if ($print_sub_title) {
			$o->openField(translate($this->pref_vars[$pref_name]['name']));
		} else {
			$o->openField('&nbsp;');
		}

			$current_conf = $this->pref_vars[$pref_name]['default'];

			$readonly = !($have_lock && $is_admin);

			if ($readonly) {
				echo array_get_index($this->html_conf_options, $current_conf);
			} else {
				combo_box($pref_name, $this->html_conf_options, FALSE, $current_conf);
			}

		$o->closeField();

	}//end paintSqCommentHtmlConfBackend()


	/**
	* Paints the interface for allowing keyword replacement
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref_name			a particular preference to paint
	* @param string		$print_sub_title	if TRUE print the sub title
	* @param boolean	$is_root			are we the root user?
	* @param boolean	$is_admin			are we an admin user?
	*
	* @return void
	* @access public
	*/
	function paintSqCommentDisableKeywordReplacementsBackend(&$o, $have_lock, $pref_name, $print_sub_title, $is_root, $is_admin)
	{
		if ($print_sub_title) {
			$o->openField(translate($this->pref_vars[$pref_name]['name']));
		} else {
			$o->openField('&nbsp;');
		}

			$current_conf = $this->pref_vars[$pref_name]['default'];

			$readonly = !($have_lock && $is_admin);

			if ($readonly) {
				echo array_get_index($this->disable_keyword_replacements, $current_conf);
			} else {
				combo_box($pref_name, $this->disable_keyword_replacements, FALSE, $current_conf);
			}

		$o->closeField();

	}//end paintSqCommentDisableKeywordReplacementBackend()


	/**
	* Paints the interface for allowing HTML in comments
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref_name			a particular preference to paint
	* @param string		$print_sub_title	if TRUE print the sub title
	* @param boolean	$is_root			are we the root user?
	* @param boolean	$is_admin			are we an admin user?
	*
	* @return void
	* @access public
	*/
	function paintSqCommentMaxRatingBackend(&$o, $have_lock, $pref_name, $print_sub_title, $is_root, $is_admin)
	{
		if ($print_sub_title) {
			$o->openField(translate($this->pref_vars[$pref_name]['name']));
		} else {
			$o->openField('&nbsp;');
		}

		$max_rating = $this->pref_vars[$pref_name]['default'];

		$readonly = !($have_lock && $is_admin);

		$this->int_max_rating->value = $max_rating;
		$this->int_max_rating->paint($pref_name, $readonly);

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
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) {
			return FALSE;
		}

		$prefs = array_keys($this->pref_vars);
		if (!is_null($pref) && in_array($pref, array_keys($this->pref_vars))) {
			$prefs = Array($pref);
		}

		$res = TRUE;

		foreach ($prefs as $pref) {
			$fn = 'process'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($pref)))).'Backend';
			if (!method_exists($this, $fn)) continue;

			// call individual process function for each pref
			if (!$this->$fn($o, $have_lock, $pref, $is_root, $is_admin)) {
				$res = FALSE;
			}
		}
		return $res;

	}//end processBackend()


	/**
	* Processes the interface for configuring the size of the comment input box
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	* @param boolean	$is_root	are we the root user?
	* @param boolean	$is_admin	are we an admin user?
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentInputSizeBackend(&$o, $have_lock, $pref, $is_root, $is_admin)
	{

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
	* @param boolean	$is_root	are we the root user?
	* @param boolean	$is_admin	are we an admin user?
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentMaxLengthBackend(&$o, $have_lock, $pref, $is_root, $is_admin)
	{
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
	* @param boolean	$is_root	are we the root user?
	* @param boolean	$is_admin	are we an admin user?
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentHtmlConfBackend(&$o, $have_lock, $pref, $is_root, $is_admin)
	{
		$new_setting = array_get_index($_POST, $pref);

		if (!is_null($new_setting)) {
			$this->pref_vars[$pref]['default'] = $new_setting;

			return TRUE;
		}
		return FALSE;

	}//end processSqCommentHtmlConfBackend()


	/**
	* Processes the interface for configuring the disable keyword replacement option
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	* @param boolean	$is_root	are we the root user?
	* @param boolean	$is_admin	are we an admin user?
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentDisableKeywordReplacementsBackend(&$o, $have_lock, $pref, $is_root, $is_admin)
	{
		$new_setting = array_get_index($_POST, $pref);

		if (!is_null($new_setting)) {
			$this->pref_vars[$pref]['default'] = $new_setting;

			return TRUE;
		}
		return FALSE;

	}//end processSqCommentDisableKeywordReplacementBackend()


	/**
	* Processes the interface for configuring the maximum comment length
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	* @param boolean	$is_root	are we the root user?
	* @param boolean	$is_admin	are we an admin user?
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentMaxRatingBackend(&$o, $have_lock, $pref, $is_root, $is_admin)
	{
		$this->int_max_rating->process($pref);
		if (!$this->int_max_rating->processed) return FALSE;

		$this->pref_vars[$pref]['default'] = $this->int_max_rating->value;

		return TRUE;

	}//end processSqCommentMaxRatingBackend()


	/**
	* Paints the interface for configuring the maximum comment length
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref_name			a particular preference to paint
	* @param string		$print_sub_title	if TRUE print the sub title
	* @param boolean	$is_root			are we the root user?
	* @param boolean	$is_admin			are we an admin user?
	*
	* @return void
	* @access public
	*/
	function paintSqCommentImgRatedBackend(&$o, $have_lock, $pref_name, $print_sub_title, $is_root, $is_admin)
	{
		if ($print_sub_title) {
			$o->openField(translate($this->pref_vars[$pref_name]['name']));
		} else {
			$o->openField('&nbsp;');
		}

		$current_setting = $this->pref_vars[$pref_name]['default'];

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
		$url->paint($pref_name, $read_only);


		if (!$read_only) {
			$o->note($this->pref_vars[$pref_name]['description']);
			$o->note('Empty means default.');
		}

		$o->closeField();

	}//end paintSqCommentImgRatedBackend()


	/**
	* Processes the interface for configuring the maximum comment length
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	* @param boolean	$is_root	are we the root user?
	* @param boolean	$is_admin	are we an admin user?
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentImgRatedBackend(&$o, $have_lock, $pref, $is_root, $is_admin)
	{
		$url = $this->url_processor;

		$url->process($pref);
		$new_setting = $url->value;

		$this->pref_vars[$pref]['default'] = $new_setting;

		return TRUE;

	}//end processSqCommentImgRatedBackend()


	/**
	* Paints the interface for configuring the maximum comment length
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref_name			a particular preference to paint
	* @param string		$print_sub_title	if TRUE print the sub title
	* @param boolean	$is_root			are we the root user?
	* @param boolean	$is_admin			are we an admin user?
	*
	* @return void
	* @access public
	*/
	function paintSqCommentImgBlankBackend(&$o, $have_lock, $pref_name, $print_sub_title, $is_root, $is_admin)
	{
		if ($print_sub_title) {
			$o->openField(translate($this->pref_vars[$pref_name]['name']));
		} else {
			$o->openField('&nbsp;');
		}

		$current_setting = $this->pref_vars[$pref_name]['default'];

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
		$url->paint($pref_name, $read_only);

		if (!$read_only) {
			$o->note($this->pref_vars[$pref_name]['description']);
			$o->note('Empty means default.');
		}

		$o->closeField();

	}//end paintSqCommentImgBlankBackend()


	/**
	* Processes the interface for configuring the maximum comment length
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param boolean	$pref		a particular preference to process
	* @param boolean	$is_root	are we the root user?
	* @param boolean	$is_admin	are we an admin user?
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentImgBlankBackend(&$o, $have_lock, $pref, $is_root, $is_admin)
	{
		$url = $this->url_processor;

		$url->process($pref);
		$new_setting = $url->value;

		$this->pref_vars[$pref]['default'] = $new_setting;

		return TRUE;

	}//end processSqCommentImgBlankBackend()


}//end class

?>
