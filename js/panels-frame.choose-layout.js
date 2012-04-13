
(function($) {
  Drupal.behaviors.panelsFrameChooseLayout = {
    attach: function(context, settings) {
      var tabs = settings.panelsFrame.chooseLayout.tabs;
      for (id in settings.panelsFrame.chooseLayout.tabs) {
        $('#' + id, context).once('panels-frame-choose-layout', function() {
          console.log(tabs);
          $(this)
            .tabs(tabs[id]);
            // .buttonset();
        });
      }
    }
  }
})(jQuery);