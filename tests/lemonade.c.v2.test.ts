import { Blockchain } from "@ibct-dev/sclest";
import { expect } from "chai";

beforeEach(async () => {
    await new Promise(resolve => setTimeout(resolve, 300));
    console.log("----------------------");
});

describe("lemonade.c v2 컨트랙트 테스트", () => {
    const debug = 1;
    const isLocal = true;
    const bc = new Blockchain({ host: "127.0.0.1", port: "8888" }); // test

    /* 테스트 변수들 */
    const manager = "lemonade.c"; // 컨트랙트 배포 계정
    const user = "test.p";
    const anotherUser = "test1.p";
    let contractTester: any;
    let ledTokenTester: any;
    let ledTester: any;
    let errorCount = 0;
    const initialize = 1000000;
    const lemInitialize = 1000000;

    describe("테스트가 시작되면, 블록체인이 연결 되어야 함", async () => {
        it("블록체인을 연결함", async () => {
            expect(await bc.checkConntection()).eq(true);
        });
    });

    if (isLocal) {
        describe("로컬블록체인이라면, 컨트랙트를 새로 배포해야 함", async () => {
            it("배포 계정을 필요한 경우 생성하고, 컨트랙트를 배포함", async () => {
                try {
                    contractTester = await bc.createAccountByCreator(
                        manager,
                        "led"
                    );
                    await contractTester.setContract("lemonade.c");
                    expect(contractTester.accountName).eq(manager);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    contractTester = await bc.getAccount(manager);
                    expect(contractTester.accountName).eq(manager);
                }
            });
            it("테스트를 위한 추가 계정들을 생성함", async () => {
                try {
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.createAccountByCreator(user, "led");
                    await bc.createAccountByCreator(anotherUser, "led");
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.getAccount(user);
                    await bc.getAccount(anotherUser);
                }
            });
            it("led.code 권한 생성", async () => {
                try {
                    const authData = {
                        threshold: 1,
                        keys: [
                            {
                                key: 'EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S',
                                weight: 1
                            }
                        ],
                        accounts: [
                            {
                                permission:
                                {
                                    actor: manager,
                                    permission: "led.code"
                                },
                                weight: 1
                            }
                        ],
                        waits: [],
                    }
                    const actionResult = await ledTester.actions.updateauth(
                        {
                            account: manager,
                            permission: "active",
                            parent: "owner",
                            auth: authData,
                        },
                        [
                            {
                                actor: manager,
                                permission: "owner",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: user,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${anotherUser} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: anotherUser,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${manager} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: manager,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${manager} 유저가 사용할 거버넌스 토큰 생성`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.create(
                        {
                            issuer: manager,
                            maximum_supply: `${lemInitialize}.0000 LEM`,
                        },
                        [
                            {
                                actor: "led.token",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot create: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`거버넌스 토큰 초기 생성`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.issue(
                        {
                            to: manager,
                            quantity: `4000.0000 LEM`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot issue: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 유저가 사용할 거버넌스 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: manager,
                            to: user,
                            quantity: `4000.0000 LEM`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
        });

        describe("openext(): 토큰을 저장할 채널 생성", async () => {
            it(`${user}가 LED 토큰을 저장할 채널을 만듬 `, async () => {
                try {
                    const actionResult = await contractTester.actions.openext(
                        {
                            user: user,
                            ext_symbol: {
                                sym: "4,LED",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot openext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.not.undefined;
            });

            it(`${user}가 LEM 토큰을 저장할 채널을 만듬 `, async () => {
                try {
                    const actionResult = await contractTester.actions.openext(
                        {
                            user: user,
                            ext_symbol: {
                                sym: "4,LEM",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot openext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
                expect(onlyCreated).to.not.undefined;
            });

            it(`${user}가 더미 토큰을 저장할 채널을 만듬 `, async () => {
                try {
                    const actionResult = await contractTester.actions.openext(
                        {
                            user: user,
                            ext_symbol: {
                                sym: "4,DUMMY",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot openext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[2];
                expect(onlyCreated).to.not.undefined;
            });
        });

        describe("closeext(): 토큰을 저장할 채널 제거", async () => {
            it(`${user}가 더미 토큰을 저장할 채널을 지움 `, async () => {
                try {
                    const actionResult = await contractTester.actions.closeext(
                        {
                            user: user,
                            to: user,
                            ext_symbol: {
                                sym: "4,DUMMY",
                                contract: "led.token"
                            },
                            memo: ""
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot closeext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 제거 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[2];
                expect(onlyCreated).to.undefined;
            });
        });

        describe("deposit() & withdraw(): 채널에 토큰을 저장하고 꺼내기", async () => {
            it(`${user}가 LED을 저장함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `500.0000 LED`,
                            memo: `deposit`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
            });
            it(`${user}가 LEM을 저장함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `600.0000 LEM`,
                            memo: `deposit`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
            });
            it(`${user}가 LEM을 꺼냄`, async () => {
                try {
                    const actionResult = await contractTester.actions.withdraw(
                        {
                            user: user,
                            to: user,
                            to_withdraw: {
                                quantity: "100.0000 LEM",
                                contract: "led.token"
                            },
                            memo: `out`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot withdraw: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
            });
            it(`${user} 계정 LEM 잔액 확인`, async () => {
                const balance = await bc.rpc.get_currency_balance('led.token', user, 'LEM');
                expect(balance[0]).to.equal(`3500.0000 LEM`);
            });
        });

        describe("Pool: 풀 생성, 유동성 공급 및 회수", async () => {
            it(`${user}가 LED-LEM풀을 생성함`, async () => {
                try {
                    const actionResult = await contractTester.actions.inittoken(
                        {
                            user: user,
                            new_symbol: `4,LEDLEM`,
                            initial_pool1: {
                                quantity: "100.0000 LED",
                                contract: "led.token"
                            },
                            initial_pool2: {
                                quantity: "100.0000 LEM",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot inittoken: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
            it(`${user} 계정 LP토큰 잔액 확인`, async () => {
                const balance = await bc.rpc.get_currency_balance('lemonade.c', user, 'LEDLEM');
                console.log(balance);
                expect(balance[0]).to.not.undefined;
            });
            it(`${user}가 LED-LEM풀을 중복생성 하는데 실패함`, async () => {
                try {
                    const actionResult = await contractTester.actions.inittoken(
                        {
                            user: user,
                            new_symbol: `4,LEDLEM`,
                            initial_pool1: {
                                quantity: "1.0000 LED",
                                contract: "led.token"
                            },
                            initial_pool2: {
                                quantity: "1.0000 LEM",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot inittoken: ${error}`
                    );
                    errorCount += 1;
                    expect(true).to.be.true;
                }
            });
            it(`${user}가 LED-LEM풀에 유동성을 공급함`, async () => {
                try {
                    const actionResult = await contractTester.actions.addliquidity(
                        {
                            user: user,
                            to_buy: "50.0000 LEDLEM",
                            max_asset1: "100.0000 LED",
                            max_asset2: "100.0000 LEM"
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
            it(`${user} 계정 LP토큰 잔액 확인`, async () => {
                const balance = await bc.rpc.get_currency_balance('lemonade.c', user, 'LEDLEM');
                console.log(balance);
                expect(balance[0]).to.not.undefined;
            });
            it(`${user}가 LED-LEM풀에 유동성을 공급하려고 하는데 돈이 없음`, async () => {
                try {
                    const actionResult = await contractTester.actions.addliquidity(
                        {
                            user: user,
                            to_buy: "5000.0000 LEDLEM",
                            max_asset1: "10000.0000 LED",
                            max_asset2: "10000.0000 LEM"
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot addliquidity: ${error}`
                    );
                    errorCount += 1;
                    expect(true).to.be.true;
                }
            });
            it(`${user}가 LED-LEM풀에 유동성을 회수함`, async () => {
                try {
                    const actionResult = await contractTester.actions.rmliquidity(
                        {
                            user: user,
                            to_sell: "50.0000 LEDLEM",
                            min_asset1: "10.0000 LED",
                            min_asset2: "10.0000 LEM"
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
            it(`${user} 계정 LP토큰 잔액 확인`, async () => {
                const balance = await bc.rpc.get_currency_balance('lemonade.c', user, 'LEDLEM');
                console.log(balance);
                expect(balance[0]).to.not.undefined;
            });
            it(`${user}가 LED-LEM풀에 유동성을 과하게 회수해서 실패함`, async () => {
                try {
                    const actionResult = await contractTester.actions.rmliquidity(
                        {
                            user: user,
                            to_sell: "5000.0000 LEDLEM",
                            min_asset1: "1000.0000 LED",
                            min_asset2: "1000.0000 LEM"
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                    expect(true).to.be.true;
                }
            });
        });

        describe("exchange(): 풀을 이용하여 토큰 스왑", async () => {
            it(`${user}가 LED를 이용하여 LEM을 획득함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `10.0000 LED`,
                            memo: `exchange/LEDLEM/5.0000 LEM`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 계정 account 테이블 확인`, async () => {
                const tableResult = await ledTokenTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`accounts\n${JSON.stringify(tableResult)}`);
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
            it(`${user}가 LED를 이용하여 LEM을 양심없게 얻으려 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `10.0000 LED`,
                            memo: `exchange/LEDLEM/10000.0000 LEM`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                    expect(true).to.be.true;
                }
            });
            it(`${user} 계정 account 테이블 확인`, async () => {
                const tableResult = await ledTokenTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`accounts\n${JSON.stringify(tableResult)}`);
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
            it(`${user}가 LEM을 이용하여 LED을 획득함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `10.0000 LEM`,
                            memo: `exchange/LEDLEM/5.0000 LED`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 계정 account 테이블 확인`, async () => {
                const tableResult = await ledTokenTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`accounts\n${JSON.stringify(tableResult)}`);
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
            it(`${user}가 LED 잔액을 이용하여 LEM을 획득함`, async () => {
                try {
                    const actionResult = await contractTester.actions.exchangeall(
                        {
                            user: user,
                            pair_token: "LEDLEM",
                            asset_in: {
                                sym: "4,LED",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot exchangeall: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 계정 account 테이블 확인`, async () => {
                const tableResult = await ledTokenTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`accounts\n${JSON.stringify(tableResult)}`);
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
        });
        describe("clmpoolreward(): 풀 예치 했으면 보상을 받자", async () => {
            it(`${user}가 LEDLEM풀의 중간보상을 받음`, async () => {
                try {
                    const actionResult = await contractTester.actions.clmpoolreward(
                        {
                            user: user,
                            pair_token_symbol: "LEDLEM",
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 계정 account 테이블 확인`, async () => {
                const tableResult = await ledTokenTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`accounts\n${JSON.stringify(tableResult)}`);
            });
            it(`stats 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.stats({
                    scope: "LEDLEM",
                });
                if (debug) console.log(`Stats\n${JSON.stringify(tableResult)}`);
                expect(tableResult[0]).to.not.undefined;
            });
        });
    }
});
