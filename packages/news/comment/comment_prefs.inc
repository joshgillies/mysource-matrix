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
													'default'		=> Array(
																		'width'		=> '300px',
																		'height'	=> '100px',
																	   ),
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_MAX_LENGTH'	=> Array(
													'default'		=> 0,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_HTML_CONF'	=> Array(
													'default'		=> 1,    // 1 = Escape HTML
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_MAX_RATING'	=> Array(
													'default'		=> 5,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_IMG_RATED'	=> Array(
													'default'		=> NULL,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_IMG_BLANK'	=> Array(
													'default'		=> NULL,
													'protected'		=> FALSE,
												   ),
						'SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS'	=> Array(
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

		$this->pref_vars['SQ_COMMENT_INPUT_SIZE']['name'] = translate('Input Box Size');

		$this->pref_vars['SQ_COMMENT_MAX_LENGTH']['name'] = translate('Maximum Comment Length');
		$this->pref_vars['SQ_COMMENT_MAX_LENGTH']['desc'] = translate('This setting limits the number of characters that may be entered in the comment input box. When the value is zero, this option will be ignored.');

		$this->pref_vars['SQ_COMMENT_HTML_CONF']['name']  = translate('HTML in comments');
		$this->pref_vars['SQ_COMMENT_HTML_CONF']['desc']  = translate('Determines whether HTML is allowed, escaped or removed.');

		$this->pref_vars['SQ_COMMENT_MAX_RATING']['name'] = translate('Maximum Rating');
		$this->pref_vars['SQ_COMMENT_MAX_RATING']['desc'] = translate('Sets the upper bound for comment ratings.').' '.
		    translate('Changing this value will affect ratings in existing comments are displayed, as comment ratings are internally stored as percentages.');

		$this->pref_vars['SQ_COMMENT_IMG_RATED']['name']  = translate('Rated Image');
		$this->pref_vars['SQ_COMMENT_IMG_RATED']['desc']  = translate('Image to be used for the "filled" portion of a comment rating. (For instance, if a rating is 3 out of 5, this will be used for the three "filled" rating images.)').' '.
		    translate('If left empty, the default image is used.');

		$this->pref_vars['SQ_COMMENT_IMG_BLANK']['name']  = translate('Blank Image');
		$this->pref_vars['SQ_COMMENT_IMG_BLANK']['desc']  = translate('Image to be used for the "unfilled" portion of a comment rating. (For instance, if a rating is 3 out of 5, this will be used for the two remaining "unfilled" rating images.)').' '.
		    translate('If left empty, the default image is used.');

		$this->pref_vars['SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS']['name'] = translate('Disable Keyword Replacements');

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
		$is_root   = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin  = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
        $have_lock = ($have_lock && $is_admin);

		$prefs = array_keys($this->pref_vars);
		if (!is_null($pref) && in_array($pref, array_keys($this->pref_vars))) {
			$prefs = Array($pref);
		}

		foreach ($prefs as $pref_value) {
			$fn = 'paint'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($pref_value)))).'Backend';
			if (!method_exists($this, $fn)) continue;

			// call individual paint function for each pref
			if (is_null($pref) || $pref == $pref_value) {
				$this->$fn($o, $have_lock, $pref);
			}
		}

	}//end paintBackend()


	/**
	* Paints the interface for configuring the size of the comment input box
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref			a particular preference to paint
	*
	* @return void
	* @access public
	*/
	function paintSqCommentInputSizeBackend(&$o, $have_lock, $pref)
	{
        $this->paintField($o, $have_lock, $pref, 'SQ_COMMENT_INPUT_SIZE', function($comment_size) use ($have_lock, $pref) {
            ?><table class="sq-backend-table" style="width: 350px;">
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Width') ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock) {
						$this->css_dimension_width->paint($pref.'_width', $have_lock, $comment_size['width']);
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
					if ($have_lock) {
						$this->css_dimension_height->paint($pref.'_height', $have_lock, $comment_size['height']);
					} else {
						echo $comment_size['height'];
					}
					?>
					</td>
				</tr>
			</table><?php

		});

	}//end paintSqCommentInputSizeBackend()


	/**
	* Paints the interface for configuring the maximum comment length
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref			a particular preference to paint
	*
	* @return void
	* @access public
	*/
	function paintSqCommentMaxLengthBackend(&$o, $have_lock, $pref)
	{
	    $this->paintIntTextField($o, $have_lock, $pref, 'SQ_COMMENT_MAX_LENGTH', 10, function($value) {
		     if ((int) $value === 0) {
		         return translate('Unlimited');
		     } else {
		         return sprintf(translate('%s character(s)', $value));
		     }
		}, 0);

	}//end paintSqCommentMaxLengthBackend()


	/**
	* Paints the interface for allowing HTML in comments
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref			a particular preference to paint
	*
	* @return void
	* @access public
	*/
	function paintSqCommentHtmlConfBackend(&$o, $have_lock, $pref)
	{
	    $options = Array(
                       0 => translate('Allow'),
                       1 => translate('Escape'),
                       2 => translate('Strip'),
                   );
        $this->paintComboField($o, $have_lock, $pref, 'SQ_COMMENT_HTML_CONF', $options);

	}//end paintSqCommentHtmlConfBackend()


	/**
	* Paints the interface for allowing keyword replacement
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref			a particular preference to paint
	*
	* @return void
	* @access public
	*/
	function paintSqCommentDisableKeywordReplacementsBackend(&$o, $have_lock, $pref)
	{
	    $options = Array(
                       1 => translate('Yes'),
                       0 => translate('No'),
                   );
        $this->paintComboField($o, $have_lock, $pref, 'SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS', $options);

	}//end paintSqCommentDisableKeywordReplacementBackend()


	/**
	* Paints the interface for allowing HTML in comments
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref			a particular preference to paint
	*
	* @return void
	* @access public
	*/
	function paintSqCommentMaxRatingBackend(&$o, $have_lock, $pref)
	{
	    $this->paintIntTextField($o, $have_lock, $pref, 'SQ_COMMENT_MAX_RATING', 10, '%s', 0);

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
        $have_lock = ($have_lock && $is_admin);

        if (!$have_lock) {
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
			if (!$this->$fn($o, $have_lock, $pref)) {
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
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentInputSizeBackend(&$o, $have_lock, $pref)
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
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentMaxLengthBackend(&$o, $have_lock, $pref)
	{
		if (is_null($pref) || $pref == 'SQ_COMMENT_MAX_LENGTH') {
			if (isset($_POST['prefs']['SQ_COMMENT_MAX_LENGTH'])) {
				$this->pref_vars['SQ_COMMENT_MAX_LENGTH']['default'] = $_POST['prefs']['SQ_COMMENT_MAX_LENGTH'];
			}
		}

		return TRUE;

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
		if (is_null($pref) || $pref == 'SQ_COMMENT_HTML_CONF') {
			if (isset($_POST['prefs']['SQ_COMMENT_HTML_CONF'])) {
				$this->pref_vars['SQ_COMMENT_HTML_CONF']['default'] = $_POST['prefs']['SQ_COMMENT_HTML_CONF'];
			}
		}

		return TRUE;

	}//end processSqCommentHtmlConfBackend()


	/**
	* Processes the interface for configuring the disable keyword replacement option
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentDisableKeywordReplacementsBackend(&$o, $have_lock, $pref)
	{
		if (is_null($pref) || $pref == 'SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS') {
			if (isset($_POST['prefs']['SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS'])) {
				$this->pref_vars['SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS']['default'] = $_POST['prefs']['SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS'];
			}
		}

		return TRUE;

	}//end processSqCommentDisableKeywordReplacementBackend()


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
		if (is_null($pref) || $pref == 'SQ_COMMENT_MAX_RATING') {
			if (isset($_POST['prefs']['SQ_COMMENT_MAX_RATING'])) {
				$this->pref_vars['SQ_COMMENT_MAX_RATING']['default'] = $_POST['prefs']['SQ_COMMENT_MAX_RATING'];
			}
		}

		return TRUE;

	}//end processSqCommentMaxRatingBackend()


	/**
	* Paints the interface for configuring the maximum comment length
	*
	* @param object		&$o					reference to the backend outputter instance
	* @param boolean	$have_lock			do we have the lock to edit these preferences or not
	* @param string		$pref			a particular preference to paint
	*
	* @return void
	* @access public
	*/
	function paintSqCommentImgRatedBackend(&$o, $have_lock, $pref)
	{
		$this->paintField($o, $have_lock, $pref, 'SQ_COMMENT_IMG_RATED', function($current_setting) use ($have_lock, $pref) {
            $image_location_prefix = sq_web_path('data').'/asset_types/comment/files';
            if (empty($current_setting)) {
                $current_image = sq_web_path('data').'/asset_types/comment/files/star.png';
            } else {
                $current_image = $current_setting;
            }

            echo sprintf(
                translate('Current Image: %s'),
                '<img src="'.htmlspecialchars($current_image).'" />'
            );
            echo '<br />';

            $url = $this->url_processor;
            $url->value = $current_setting;
            $url->paint($pref, !$have_lock);
		});

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
	* @param string		$pref			a particular preference to paint
	*
	* @return void
	* @access public
	*/
	function paintSqCommentImgBlankBackend(&$o, $have_lock, $pref)
	{
		$this->paintField($o, $have_lock, $pref, 'SQ_COMMENT_IMG_BLANK', function($current_setting) use ($have_lock, $pref) {
            $image_location_prefix = sq_web_path('data').'/asset_types/comment/files';
            if (empty($current_setting)) {
                $current_image = sq_web_path('data').'/asset_types/comment/files/star_grey.png';
            } else {
                $current_image = $current_setting;
            }

            echo sprintf(
                translate('Current Image: %s'),
                '<img src="'.htmlspecialchars($current_image).'" />'
            );
            echo '<br />';

            $url = $this->url_processor;
            $url->value = $current_setting;
            $url->paint($pref, !$have_lock);
		});

	}//end paintSqCommentImgBlankBackend()


	/**
	* Processes the interface for configuring the maximum comment length
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param boolean	$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processSqCommentImgBlankBackend(&$o, $have_lock, $pref)
	{
		$url = $this->url_processor;

		$url->process($pref);
		$new_setting = $url->value;

		$this->pref_vars[$pref]['default'] = $new_setting;

		return TRUE;

	}//end processSqCommentImgBlankBackend()


}//end class

?>
