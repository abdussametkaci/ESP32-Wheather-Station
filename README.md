# ESP32-Wheather-Station
## Kullanılan Sensörler
1) ESP32 DOIT Devkit V1
2) CJMCU-8118+CCS811+HDC1080 Sensör kartı
3) Mikro SD KArt Modülü
## Kullanım Amacı
Sensör kartından gelen sıcaklık, co2, tvoc ve nem gibi verileri sd karta kaydetmek.
Ayrıca ESP32'de yazılmış olan server sayesinde browser'dan uygulamamıza girip verilerimizi
bir tabloda ve grafikte görebilmekteyiz. Ayrıca istediğimiz zaman download butonuna basarak
sd kartta kaydedilmiş verileri .csv formatında indirilebilir.
## Konfigürasyon
Sd kart modülü içerisinde config.txt adında bir dosya bulumalıdır. Bu dosya birkaç durum için
cihazınızı konfigüre etmenizi sağlar. İçinde JSON formatında yazılmış veriler vardır. Bu veriler; 
1) has_wifi: Boolean tipinde veridir.
2) wpa2_enterprise: Boolean tipinde veridir. Eğer cihazınızı koyduğunuz mekanda wifi varsa ancak bu wifi
kullanıcı adı ve şifre ile giriş yapılması gerekiyorsa eğer bu değeri true yapmalısınız. Eğer normal bir
wifi ağınız varsa yani sadece şifre ile giriş yapılıyorsa bu değeri false yapmalısınız.
3) ssid: String tipinde bir veridir. Giriş yapacağınız veya wifi olmadığı durunlarda yayacağı internet ağının
adını belirlemektedir. Eğer wifi ağınız varsa giriş yapacağınız wifi ağının adını yazmalısınız.
4) password: String tipinde bir veridir. Giriş yapacağınız wifi ağının şifresidir. Wifi ağı yok ise, yayacağınız
ağın şifresini belirleyebilirsiniz.
5) username: String tipinde bir veridir. Wifi ağına bağlanırken kullanıcı adı ve şifre ile giriş
yapmanız gerekiyorsa eğer (WPA2 Enterprise), bu özelliği kullanmalısınız. Kullanıcı adınızı belirler.
6) hostname: String tipinde bir veridir. Wifi ağınız olsun veya olmasın,bağlandığınız cihaza browser'da
hangi adres ile bağlnabileceğini belirler. Örneğin buraya esp32 yazarsanız eğer, siz http://esp32.local
web adresinden cihazınıza ulaşabilirsiniz. (NOT: Aynı ağa bağlı olmanız gerekmekyedir.)
## Web Sayfası
ESP32 geliştirme katına SPIIFFS sayesinde geliştirdiğimzi web sayfasını ESP32'nin falsh memory'sine 
yazabilmekteyiz. Bunu yapabilmek için öncelikle SPIFFS'i bilgisayarınıza kurmanız gerekmektedir. Sonrasında
Arduino IDE'sine upload sketch bir tool gelecek ve bu tool sayesinde web sayfasını ESP32'nin falsh'ına atabilirsiniz.
Web sayfasını karta yüklemek için Arduino projesini geliştirdiğiniz klasörde 'data' adında bir klasör açınız. Bu klasörün
adını değiştiremezsiniz 'data' olmak zorunda. Daha sonra geliştirdiğiniz web sayfasını bu klasörün içine atıp karta yükleyebilisiniz.
Bu projede grafikleri çizdirmek için highcharts sitesinden paket çektim ancak bu grafikleri çizdirebilmeniz için internete bağlı olması gerekmektedir.
Eğer internetiniz yoksa bunu çizidremezsiniz ancak bu durum için bir tane web sayfası tasarladım ve burada sadce tablo ve mikro sd kart modülündeki
verileri yükleyebileceğiniz bir adet buton bulunmaktadır.

Geliştirdiğiniz web sayfasının boyutuna dikkat etmelisiniz. 1 MB ila 3 MB arasında yüklemeye izin vermektedir. Web sayfası çok yer kaplarsa
problem olabilir. Tavsiye olarak eğer büyük bir web uygulamnız var ise bunu mikro sd kart modülüne yükleyip kullanıcı sizin adresinize istek attığında
sd karttan bu web sayfasını çekip kullanıcıya gönderebilrisiniz.
## SD Kart Hakkında
ESP32 geliştirme kartı ile uğraşırken mikro SD kart modülü yazma problemleri yaşanabilmektedir. Eğer yazma hakkında problem yaşıyorsanız dosyanızı açmadan hemen önce SD kart modülünü yeniden başlatmanız gerekmektedir (SD.end();SD.begin()). Eğer yaşamıyorsanız bunu yapmk gerekmez. Ancak serverdan SD kart içerisindeki dosyayı indirmek isterseniz bunu indirmeden hemen önce yapmanız gerekmektedir.
