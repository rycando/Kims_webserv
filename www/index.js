$("document").ready(function () {
	$("#team").click(function (event) {
		event.preventDefault();
		$("html,body").animate({ scrollTop: $("#sec_team").offset().top }, 300);
	});
	$("#document").click(function (event) {
		event.preventDefault();
		$("html,body").animate(
			{ scrollTop: $("#sec_document").offset().top },
			300
		);
	});
	$("#errors").click(function (event) {
		event.preventDefault();
		$("html,body").animate(
			{ scrollTop: $("#sec_errors").offset().top },
			300
		);
	});
});
